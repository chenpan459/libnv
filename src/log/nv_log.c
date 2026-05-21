#include "nv_log.h"

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define NV_LOG_MSG_MAX  NV_LOG_LINE_MAX

typedef struct {
    _Atomic uint64_t  seq;
    nv_log_level_e    level;
    char              text[NV_LOG_MSG_MAX];
} nv_log_slot_t;

typedef struct {
    nv_log_slot_t    *slots;
    size_t            capacity;
    size_t            mask;

    _Atomic uint64_t  enqueue_pos;
    _Atomic uint64_t  dequeue_pos;

    pthread_t         thread;
    pthread_mutex_t   wait_mutex;
    pthread_cond_t    not_empty;
    int               thread_started;
    _Atomic int       stop;
} nv_log_ring_t;

static const char *const log_info[NV_LOG_LEVEL_MAX] = {
    "DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "CRIT", "FATAL",
};

static char                g_log_module[32] = "NV";
static _Atomic int         g_log_level     = NV_LOG_LEVEL_INFO;
static _Atomic int         g_log_console   = 1;
static _Atomic int         g_log_syslog    = 0;
static pthread_mutex_t     g_module_mutex  = PTHREAD_MUTEX_INITIALIZER;
static FILE               *g_log_file        = NULL;

static nv_log_ring_t       g_ring;
static size_t              g_ring_capacity   = NV_LOG_QUEUE_DEFAULT;
static nv_log_overflow_e   g_overflow_policy = NV_LOG_OVERFLOW_DROP;
static _Atomic uint64_t    g_dropped         = 0;
static _Atomic int         g_async_ready     = 0;
static size_t              g_rotate_max_bytes = 0;
static int                 g_rotate_keep      = 0;
static char                g_rotate_path[512];

static long nv_log_get_tid(void)
{
    return (long)syscall(SYS_gettid);
}

static int nv_log_to_syslog_priority(nv_log_level_e level)
{
    switch (level) {
    case NV_LOG_LEVEL_DEBUG:   return LOG_DEBUG;
    case NV_LOG_LEVEL_INFO:    return LOG_INFO;
    case NV_LOG_LEVEL_NOTICE:  return LOG_NOTICE;
    case NV_LOG_LEVEL_WARNING: return LOG_WARNING;
    case NV_LOG_LEVEL_ERROR:   return LOG_ERR;
    case NV_LOG_LEVEL_CRIT:    return LOG_CRIT;
    case NV_LOG_LEVEL_FATAL:   return LOG_ALERT;
    default:                   return LOG_INFO;
    }
}

static size_t nv_log_round_pow2(size_t n)
{
    size_t p = NV_LOG_QUEUE_MIN;

    if (n >= NV_LOG_QUEUE_MAX) {
        return NV_LOG_QUEUE_MAX;
    }
    while (p < n) {
        p <<= 1;
    }
    return p;
}

static void nv_log_copy_module(char *dst, size_t dst_size)
{
    memcpy(dst, g_log_module, sizeof(g_log_module));
    dst[dst_size - 1] = '\0';
}

void nv_log_set_module(const char *module)
{
    pthread_mutex_lock(&g_module_mutex);
    if (!module || module[0] == '\0') {
        snprintf(g_log_module, sizeof(g_log_module), "NV");
    } else {
        snprintf(g_log_module, sizeof(g_log_module), "%s", module);
    }
    pthread_mutex_unlock(&g_module_mutex);
}

const char *nv_log_get_module(void)
{
    return g_log_module;
}

void nv_log_set_console(int enable)
{
    atomic_store_explicit(&g_log_console, enable ? 1 : 0, memory_order_release);
}

int nv_log_get_console(void)
{
    return atomic_load_explicit(&g_log_console, memory_order_acquire);
}

void nv_log_set_level(nv_log_level_e level)
{
    if (level < 0) {
        level = NV_LOG_LEVEL_DEBUG;
    } else if (level >= NV_LOG_LEVEL_MAX) {
        level = NV_LOG_LEVEL_FATAL;
    }
    atomic_store_explicit(&g_log_level, (int)level, memory_order_release);
}

int nv_log_get_level(void)
{
    return atomic_load_explicit(&g_log_level, memory_order_acquire);
}

const char *nv_log_get_info(nv_log_level_e level)
{
    if (level < 0 || level >= NV_LOG_LEVEL_MAX) {
        return "UNKNOWN";
    }
    return log_info[level];
}

int nv_log_level_from_string(const char *name, nv_log_level_e *out)
{
    if (!name || !out) {
        return -1;
    }

    if (strcasecmp(name, "debug") == 0 || strcasecmp(name, "dump") == 0) {
        *out = NV_LOG_LEVEL_DEBUG;
    } else if (strcasecmp(name, "info") == 0) {
        *out = NV_LOG_LEVEL_INFO;
    } else if (strcasecmp(name, "notice") == 0) {
        *out = NV_LOG_LEVEL_NOTICE;
    } else if (strcasecmp(name, "warning") == 0 || strcasecmp(name, "warn") == 0) {
        *out = NV_LOG_LEVEL_WARNING;
    } else if (strcasecmp(name, "error") == 0) {
        *out = NV_LOG_LEVEL_ERROR;
    } else if (strcasecmp(name, "crit") == 0 || strcasecmp(name, "critical") == 0) {
        *out = NV_LOG_LEVEL_CRIT;
    } else if (strcasecmp(name, "fatal") == 0) {
        *out = NV_LOG_LEVEL_FATAL;
    } else {
        char *end = NULL;
        long  v   = strtol(name, &end, 0);
        if (end == name || v < 0 || v >= NV_LOG_LEVEL_MAX) {
            return -1;
        }
        *out = (nv_log_level_e)v;
    }
    return 0;
}

int nv_log_overflow_from_string(const char *name, nv_log_overflow_e *out)
{
    if (!name || !out) {
        return -1;
    }
    if (strcasecmp(name, "drop") == 0) {
        *out = NV_LOG_OVERFLOW_DROP;
    } else if (strcasecmp(name, "block") == 0) {
        *out = NV_LOG_OVERFLOW_BLOCK;
    } else if (strcasecmp(name, "overwrite") == 0 ||
               strcasecmp(name, "cover") == 0) {
        *out = NV_LOG_OVERFLOW_OVERWRITE;
    } else {
        return -1;
    }
    return 0;
}

void nv_log_set_queue_size(size_t capacity)
{
    if (atomic_load_explicit(&g_async_ready, memory_order_acquire)) {
        return;
    }
    g_ring_capacity = nv_log_round_pow2(capacity);
}

size_t nv_log_get_queue_size(void)
{
    return g_ring.capacity ? g_ring.capacity : g_ring_capacity;
}

void nv_log_set_overflow_policy(nv_log_overflow_e policy)
{
    if (policy < NV_LOG_OVERFLOW_DROP || policy > NV_LOG_OVERFLOW_OVERWRITE) {
        return;
    }
    g_overflow_policy = policy;
}

nv_log_overflow_e nv_log_get_overflow_policy(void)
{
    return g_overflow_policy;
}

uint64_t nv_log_get_dropped_count(void)
{
    return atomic_load_explicit(&g_dropped, memory_order_relaxed);
}

void nv_log_get_queue_stats(nv_log_queue_stats_t *stats)
{
    uint64_t en;
    uint64_t de;

    if (!stats) {
        return;
    }

    memset(stats, 0, sizeof(*stats));
    stats->dropped = atomic_load_explicit(&g_dropped, memory_order_relaxed);

    if (!g_ring.capacity) {
        stats->capacity = g_ring_capacity;
        return;
    }

    en = atomic_load_explicit(&g_ring.enqueue_pos, memory_order_acquire);
    de = atomic_load_explicit(&g_ring.dequeue_pos, memory_order_acquire);
    stats->capacity = g_ring.capacity;
    stats->pending  = (size_t)(en - de);
    switch (g_overflow_policy) {
    case NV_LOG_OVERFLOW_BLOCK:   stats->overflow = "block"; break;
    case NV_LOG_OVERFLOW_OVERWRITE: stats->overflow = "overwrite"; break;
    default:                    stats->overflow = "drop"; break;
    }
}

static int nv_log_open_default_file(void)
{
    time_t     now = time(NULL);
    struct tm  tm_buf;
    struct tm *t;
    char       time_str[64];
    char       filename[128];
    FILE      *fp;

    t = localtime_r(&now, &tm_buf);
    if (!t) {
        return -1;
    }

    snprintf(time_str, sizeof(time_str), "%04d%02d%02d-%02d%02d%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    snprintf(filename, sizeof(filename), "/tmp/nv%slog", time_str);

    fp = fopen(filename, "a");
    if (!fp) {
        return -1;
    }

    if (g_log_file) {
        fclose(g_log_file);
    }
    g_log_file = fp;
    return 0;
}

void nv_log_set_rotate(size_t max_bytes, int keep_files)
{
    g_rotate_max_bytes = max_bytes;
    g_rotate_keep      = keep_files > 0 ? keep_files : 1;
}

static void nv_log_remember_path(const char *path)
{
    if (!path || !path[0]) {
        g_rotate_path[0] = '\0';
        return;
    }
    snprintf(g_rotate_path, sizeof(g_rotate_path), "%s", path);
}

static void nv_log_rotate_if_needed(void)
{
    struct stat st;
    char        oldpath[560];
    char        newpath[560];
    int         i;

    if (g_rotate_max_bytes == 0 || g_rotate_path[0] == '\0' || !g_log_file) {
        return;
    }
    if (stat(g_rotate_path, &st) != 0 || (size_t)st.st_size < g_rotate_max_bytes) {
        return;
    }

    fflush(g_log_file);
    fclose(g_log_file);
    g_log_file = NULL;

    snprintf(oldpath, sizeof(oldpath), "%s.%d", g_rotate_path, g_rotate_keep);
    remove(oldpath);
    for (i = g_rotate_keep - 1; i >= 1; i--) {
        snprintf(oldpath, sizeof(oldpath), "%s.%d", g_rotate_path, i);
        snprintf(newpath, sizeof(newpath), "%s.%d", g_rotate_path, i + 1);
        rename(oldpath, newpath);
    }
    snprintf(newpath, sizeof(newpath), "%s.1", g_rotate_path);
    rename(g_rotate_path, newpath);

    g_log_file = fopen(g_rotate_path, "a");
}

static void nv_log_ensure_file_consumer(void)
{
    if (g_log_file == NULL) {
        nv_log_open_default_file();
    }
    nv_log_rotate_if_needed();
}

static void nv_log_write_msg(const nv_log_slot_t *slot)
{
    int   console;
    int   use_syslog;
    FILE *fp;

    if (!slot || !slot->text[0]) {
        return;
    }

    console    = nv_log_get_console();
    use_syslog = atomic_load_explicit(&g_log_syslog, memory_order_acquire);

    if (console) {
        fputs(slot->text, stderr);
        fflush(stderr);
    }

    nv_log_ensure_file_consumer();
    fp = g_log_file;
    if (fp) {
        fputs(slot->text, fp);
        fflush(fp);
    }

    if (use_syslog) {
        char   syslog_buf[NV_LOG_MSG_MAX];
        size_t n = strlen(slot->text);

        if (n >= sizeof(syslog_buf)) {
            n = sizeof(syslog_buf) - 1;
        }
        memcpy(syslog_buf, slot->text, n);
        syslog_buf[n] = '\0';
        if (n > 0 && syslog_buf[n - 1] == '\n') {
            syslog_buf[n - 1] = '\0';
        }
        syslog(nv_log_to_syslog_priority(slot->level), "%s", syslog_buf);
    }
}

static int nv_log_ring_pop(nv_log_slot_t *out)
{
    nv_log_ring_t *r = &g_ring;
    uint64_t       pos;
    uint64_t       tail;
    nv_log_slot_t *slot;
    uint64_t       seq;

    pos = atomic_load_explicit(&r->dequeue_pos, memory_order_relaxed);
    tail = atomic_load_explicit(&r->enqueue_pos, memory_order_acquire);
    if (pos >= tail) {
        return 0;
    }

    slot = &r->slots[pos & r->mask];
    seq  = atomic_load_explicit(&slot->seq, memory_order_acquire);
    if (seq != pos + 1) {
        return 0;
    }

    if (out) {
        memcpy(out->text, slot->text, sizeof(out->text));
        out->level = slot->level;
    }

    atomic_store_explicit(&slot->seq,
                          pos + r->capacity,
                          memory_order_release);
    atomic_store_explicit(&r->dequeue_pos, pos + 1, memory_order_release);
    return 1;
}

static void nv_log_ring_signal_consumer(void)
{
    pthread_mutex_lock(&g_ring.wait_mutex);
    pthread_cond_signal(&g_ring.not_empty);
    pthread_mutex_unlock(&g_ring.wait_mutex);
}

static int nv_log_ring_push(const char *line, nv_log_level_e level)
{
    nv_log_ring_t *r = &g_ring;
    size_t         len;
    uint64_t       pos;
    uint64_t       head;
    uint64_t       tail;
    nv_log_slot_t *slot;
    int            spins;

    if (!line || !atomic_load_explicit(&g_async_ready, memory_order_acquire)) {
        atomic_fetch_add_explicit(&g_dropped, 1, memory_order_relaxed);
        return -1;
    }

    len = strlen(line);
    if (len == 0) {
        return 0;
    }
    if (len >= NV_LOG_MSG_MAX) {
        len = NV_LOG_MSG_MAX - 1;
    }

    for (;;) {
        pos  = atomic_load_explicit(&r->enqueue_pos, memory_order_relaxed);
        head = atomic_load_explicit(&r->dequeue_pos, memory_order_acquire);

        if (pos - head >= r->capacity) {
            if (g_overflow_policy == NV_LOG_OVERFLOW_DROP ||
                g_overflow_policy == NV_LOG_OVERFLOW_BLOCK) {
                atomic_fetch_add_explicit(&g_dropped, 1, memory_order_relaxed);
                return -1;
            }
            /* OVERWRITE: 丢弃最旧一条，推进 dequeue */
            if (!atomic_compare_exchange_weak_explicit(
                    &r->dequeue_pos, &head, head + 1,
                    memory_order_acq_rel, memory_order_relaxed)) {
                continue;
            }
            atomic_fetch_add_explicit(&g_dropped, 1, memory_order_relaxed);
            slot = &r->slots[head & r->mask];
            atomic_store_explicit(&slot->seq,
                                  head + r->capacity,
                                  memory_order_release);
            continue;
        }

        if (atomic_compare_exchange_weak_explicit(
                &r->enqueue_pos, &pos, pos + 1,
                memory_order_acq_rel, memory_order_relaxed)) {
            break;
        }
    }

    slot = &r->slots[pos & r->mask];
    for (spins = 0; spins < 10000; spins++) {
        uint64_t seq = atomic_load_explicit(&slot->seq, memory_order_acquire);
        if (seq == pos) {
            break;
        }
        if (spins > 64) {
            sched_yield();
        }
    }
    if (atomic_load_explicit(&slot->seq, memory_order_acquire) != pos) {
        atomic_fetch_add_explicit(&g_dropped, 1, memory_order_relaxed);
        return -1;
    }

    memcpy(slot->text, line, len);
    slot->text[len] = '\0';
    slot->level     = level;
    atomic_store_explicit(&slot->seq, pos + 1, memory_order_release);

    tail = atomic_load_explicit(&r->enqueue_pos, memory_order_acquire);
    if (atomic_load_explicit(&r->dequeue_pos, memory_order_acquire) < tail) {
        nv_log_ring_signal_consumer();
    }
    return 0;
}

static void *nv_log_thread_main(void *arg)
{
    nv_log_slot_t msg;

    (void)arg;

#if defined(__linux__)
    (void)pthread_setname_np(pthread_self(), "nv-log");
#endif

    for (;;) {
        if (nv_log_ring_pop(&msg)) {
            nv_log_write_msg(&msg);
            continue;
        }

        if (atomic_load_explicit(&g_ring.stop, memory_order_acquire)) {
            break;
        }

        pthread_mutex_lock(&g_ring.wait_mutex);
        if (!nv_log_ring_pop(&msg)) {
            if (!atomic_load_explicit(&g_ring.stop, memory_order_acquire)) {
                pthread_cond_wait(&g_ring.not_empty, &g_ring.wait_mutex);
            }
        }
        pthread_mutex_unlock(&g_ring.wait_mutex);

        if (nv_log_ring_pop(&msg)) {
            nv_log_write_msg(&msg);
        } else if (atomic_load_explicit(&g_ring.stop, memory_order_acquire)) {
            break;
        }
    }

    while (nv_log_ring_pop(&msg)) {
        nv_log_write_msg(&msg);
    }

    return NULL;
}

static int nv_log_ring_init(void)
{
    nv_log_ring_t *r = &g_ring;
    size_t         i;

    if (atomic_load_explicit(&g_async_ready, memory_order_acquire)) {
        return 0;
    }

    memset(r, 0, sizeof(*r));
    r->capacity = nv_log_round_pow2(g_ring_capacity);
    r->mask     = r->capacity - 1;
    r->slots    = calloc(r->capacity, sizeof(nv_log_slot_t));
    if (!r->slots) {
        return -1;
    }

    for (i = 0; i < r->capacity; i++) {
        atomic_init(&r->slots[i].seq, i);
    }
    atomic_init(&r->enqueue_pos, 0);
    atomic_init(&r->dequeue_pos, 0);
    atomic_init(&r->stop, 0);

    pthread_mutex_init(&r->wait_mutex, NULL);
    pthread_cond_init(&r->not_empty, NULL);

    if (pthread_create(&r->thread, NULL, nv_log_thread_main, NULL) != 0) {
        pthread_mutex_destroy(&r->wait_mutex);
        pthread_cond_destroy(&r->not_empty);
        free(r->slots);
        r->slots = NULL;
        return -1;
    }

    r->thread_started = 1;
    atomic_store_explicit(&g_async_ready, 1, memory_order_release);
    return 0;
}

static int nv_log_async_start(void)
{
    return nv_log_ring_init();
}

static void nv_log_append_newline(char *buf, size_t size)
{
    size_t len = strlen(buf);
    if (len == 0 || buf[len - 1] != '\n') {
        if (len + 1 < size) {
            buf[len]     = '\n';
            buf[len + 1] = '\0';
        }
    }
}

static int nv_log_format_header(char *buf, size_t size, nv_log_level_e level,
                                const char *module, const char *file, int line,
                                const char *module_fallback)
{
    struct timeval tv;
    struct tm      tm_buf;
    struct tm     *tm_ptr;
    const char    *mod;

    gettimeofday(&tv, NULL);
    tm_ptr = localtime_r(&tv.tv_sec, &tm_buf);
    if (!tm_ptr) {
        return -1;
    }

    mod = (module && module[0]) ? module : module_fallback;

    return snprintf(buf, size,
                    "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [%s] [PID:%d TID:%ld] [%s] %s:%d ",
                    tm_ptr->tm_year + 1900,
                    tm_ptr->tm_mon + 1,
                    tm_ptr->tm_mday,
                    tm_ptr->tm_hour,
                    tm_ptr->tm_min,
                    tm_ptr->tm_sec,
                    (int)(tv.tv_usec / 1000),
                    nv_log_get_info(level),
                    (int)getpid(),
                    nv_log_get_tid(),
                    mod,
                    filename(file),
                    line);
}

/* 进程加载时启动日志线程，保证业务线程首次写日志前队列已就绪 */
__attribute__((constructor(101)))
static void nv_log_auto_start(void)
{
    (void)nv_log_async_start();
}

int nv_log_init_file(const char *path)
{
    FILE *fp = NULL;

    if (nv_log_async_start() != 0) {
        return -1;
    }

    if (!path || path[0] == '\0') {
        return nv_log_open_default_file();
    }

    fp = fopen(path, "a");
    if (!fp) {
        return -1;
    }
    if (g_log_file) {
        fclose(g_log_file);
    }
    g_log_file = fp;
    nv_log_remember_path(path);
    return 0;
}

int nv_log_init_syslog(const char *ident)
{
    openlog(ident ? ident : "nv", LOG_PID | LOG_CONS, LOG_DAEMON);
    atomic_store_explicit(&g_log_syslog, 1, memory_order_release);
    return 0;
}

int nv_log_init(void)
{
    if (nv_log_async_start() != 0) {
        return -1;
    }
    return nv_log_open_default_file();
}

void nv_log_emit(nv_log_level_e level, const char *module,
                 const char *file, int line, const char *format, ...)
{
    char    line_buf[NV_LOG_MSG_MAX];
    char    mod_buf[32];
    va_list ap;
    int     header_len;

    if (level < 0 || level >= NV_LOG_LEVEL_MAX) {
        return;
    }

    if (nv_log_get_level() > (int)level) {
        return;
    }

    nv_log_copy_module(mod_buf, sizeof(mod_buf));

    header_len = nv_log_format_header(line_buf, sizeof(line_buf),
                                      level, module, file, line, mod_buf);
    if (header_len < 0 || (size_t)header_len >= sizeof(line_buf)) {
        return;
    }

    va_start(ap, format);
    vsnprintf(line_buf + header_len,
              sizeof(line_buf) - (size_t)header_len,
              format, ap);
    va_end(ap);

    nv_log_append_newline(line_buf, sizeof(line_buf));
    (void)nv_log_ring_push(line_buf, level);
}

void nv_log_write(const char *format, ...)
{
    char    buf[NV_LOG_MSG_MAX];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    (void)nv_log_ring_push(buf, NV_LOG_LEVEL_INFO);
}

static void nv_log_ring_shutdown(void)
{
    nv_log_ring_t *r = &g_ring;

    if (!r->thread_started) {
        return;
    }

    atomic_store_explicit(&r->stop, 1, memory_order_release);
    pthread_mutex_lock(&r->wait_mutex);
    pthread_cond_broadcast(&r->not_empty);
    pthread_mutex_unlock(&r->wait_mutex);
    pthread_join(r->thread, NULL);

    pthread_mutex_destroy(&r->wait_mutex);
    pthread_cond_destroy(&r->not_empty);
    free(r->slots);
    memset(r, 0, sizeof(*r));
    atomic_store_explicit(&g_async_ready, 0, memory_order_release);
}

void nv_log_close(void)
{
    nv_log_ring_shutdown();

    if (g_log_file != NULL) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

void nv_log_hex_dump(const uint8_t *data, size_t len)
{
    char   chunk[NV_LOG_MSG_MAX];
    char   line[128];
    size_t i;
    size_t j;
    int    pos;

    if (!data || len == 0) {
        return;
    }

    for (i = 0; i < len; ) {
        pos      = 0;
        chunk[0] = '\0';
        while (i < len && pos < (int)sizeof(chunk) - 128) {
            int lp = snprintf(line, sizeof(line), "  ");
            for (j = i; j < i + 16 && j < len; j++) {
                lp += snprintf(line + lp, sizeof(line) - (size_t)lp,
                               "%02X ", data[j]);
            }
            lp += snprintf(line + lp, sizeof(line) - (size_t)lp, "\n");
            pos += snprintf(chunk + pos, sizeof(chunk) - (size_t)pos, "%s", line);
            i += 16;
        }
        if (chunk[0]) {
            (void)nv_log_ring_push(chunk, NV_LOG_LEVEL_DEBUG);
        }
    }
}

void printf_hex(const uint8_t *data, size_t len)
{
    nv_log_hex_dump(data, len);
}
