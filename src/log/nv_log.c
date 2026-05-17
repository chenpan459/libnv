#include "nv_log.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

static pthread_mutex_t g_nv_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *const log_info[NV_LOG_LEVEL_MAX] = {
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "CRIT",
    "FATAL",
};

static char g_log_module[32] = "NV";
static int  nv_log_level     = NV_LOG_LEVEL_INFO;
static int  log_to_console   = 1;
static FILE *log_file        = NULL;
static int  log_use_syslog   = 0;

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

void nv_log_set_module(const char *module)
{
    if (!module || module[0] == '\0') {
        snprintf(g_log_module, sizeof(g_log_module), "NV");
        return;
    }
    snprintf(g_log_module, sizeof(g_log_module), "%s", module);
}

const char *nv_log_get_module(void)
{
    return g_log_module;
}

void nv_log_set_console(int enable)
{
    log_to_console = enable ? 1 : 0;
}

int nv_log_get_console(void)
{
    return log_to_console;
}

void nv_log_set_level(nv_log_level_e level)
{
    if (level < 0) {
        level = NV_LOG_LEVEL_DEBUG;
    } else if (level >= NV_LOG_LEVEL_MAX) {
        level = NV_LOG_LEVEL_FATAL;
    }
    nv_log_level = level;
}

int nv_log_get_level(void)
{
    return nv_log_level;
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
        long v = strtol(name, &end, 0);
        if (end == name || v < 0 || v >= NV_LOG_LEVEL_MAX) {
            return -1;
        }
        *out = (nv_log_level_e)v;
    }
    return 0;
}

int nv_log_init_file(const char *path)
{
    if (!path || path[0] == '\0') {
        return nv_log_init();
    }
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    log_file = fopen(path, "a");
    return (log_file == NULL) ? -1 : 0;
}

int nv_log_init_syslog(const char *ident)
{
    openlog(ident ? ident : "nv", LOG_PID | LOG_CONS, LOG_DAEMON);
    log_use_syslog = 1;
    return 0;
}

int nv_log_init(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[64] = {0};
    char filename[128] = {0};

    sprintf(time_str, "%04d%02d%02d-%02d%02d%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    sprintf(filename, "/tmp/nv%slog", time_str);

    log_file = fopen(filename, "a");
    return (log_file == NULL) ? -1 : 0;
}

static void nv_log_ensure_file(void)
{
    if (log_file == NULL) {
        nv_log_init();
    }
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
                                const char *module, const char *file, int line)
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

    mod = (module && module[0]) ? module : g_log_module;

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

void nv_log_emit(nv_log_level_e level, const char *module,
                 const char *file, int line, const char *format, ...)
{
    char    line_buf[4096];
    va_list ap;
    int     header_len;

    if (level < 0 || level >= NV_LOG_LEVEL_MAX) {
        return;
    }

    if (nv_log_get_level() > (int)level) {
        return;
    }

    header_len = nv_log_format_header(line_buf, sizeof(line_buf),
                                      level, module, file, line);
    if (header_len < 0 || (size_t)header_len >= sizeof(line_buf)) {
        return;
    }

    va_start(ap, format);
    vsnprintf(line_buf + header_len,
              sizeof(line_buf) - (size_t)header_len,
              format, ap);
    va_end(ap);

    nv_log_append_newline(line_buf, sizeof(line_buf));

    pthread_mutex_lock(&g_nv_log_mutex);

    if (log_to_console) {
        fputs(line_buf, stderr);
        fflush(stderr);
    }

    nv_log_ensure_file();
    if (log_file) {
        fputs(line_buf, log_file);
        fflush(log_file);
    }

    if (log_use_syslog) {
        size_t n = strlen(line_buf);
        if (n > 0 && line_buf[n - 1] == '\n') {
            line_buf[n - 1] = '\0';
        }
        syslog(nv_log_to_syslog_priority(level), "%s", line_buf);
    }

    pthread_mutex_unlock(&g_nv_log_mutex);
}

void nv_log_write(const char *format, ...)
{
    va_list ap;

    pthread_mutex_lock(&g_nv_log_mutex);

    nv_log_ensure_file();

    va_start(ap, format);

    if (log_to_console) {
        vfprintf(stderr, format, ap);
        fflush(stderr);
    }

    if (log_file) {
        va_list ap2;
        va_copy(ap2, ap);
        vfprintf(log_file, format, ap2);
        va_end(ap2);
        fflush(log_file);
    }

    if (log_use_syslog) {
        char buf[1024];
        va_list ap3;
        va_copy(ap3, ap);
        vsnprintf(buf, sizeof(buf), format, ap3);
        va_end(ap3);
        syslog(LOG_INFO, "%s", buf);
    }

    va_end(ap);
    pthread_mutex_unlock(&g_nv_log_mutex);
}

void nv_log_close(void)
{
    pthread_mutex_lock(&g_nv_log_mutex);
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_unlock(&g_nv_log_mutex);
}

void printf_hex(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        printf("0x%02X ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (len % 16 != 0) {
        printf("\n");
    }
}
