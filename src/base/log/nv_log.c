#include "nv_log.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_mutex_t g_nv_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *log_info[] = {
    "Dump",
    "Debug",
    "Info",
    "Warn",
    "Error",
};

char log_prex[128];
static int nv_log_level = NV_LOG_LEVEL_INFO;
static int log_to_console = 1;
static FILE *log_file = NULL;
static int log_use_syslog = 0;

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
    nv_log_level = level;
}

int nv_log_get_level(void)
{
    return nv_log_level;
}

char *nv_log_get_info(nv_log_level_e level)
{
    return log_info[level];
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

void nv_log_emit(nv_log_level_e level, const char *file, int line,
                 const char *format, ...)
{
    char    line_buf[2048];
    char    time_str[64];
    time_t  now;
    struct tm *t;
    va_list ap;

    if (nv_log_get_level() > (int)level) {
        return;
    }

    now = time(NULL);
    t   = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    snprintf(line_buf, sizeof(line_buf),
             "[ %s %s:%d#%s ] : ",
             nv_log_get_info(level),
             filename(file),
             line,
             time_str);

    va_start(ap, format);
    vsnprintf(line_buf + strlen(line_buf),
              sizeof(line_buf) - strlen(line_buf),
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
        syslog(LOG_INFO, "%s", line_buf);
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
