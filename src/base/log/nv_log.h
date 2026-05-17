#ifndef _NV_LOG_H_INCLUDED_
#define _NV_LOG_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

typedef enum
{
    NV_LOG_LEVEL_DUMP = 0,
    NV_LOG_LEVEL_DEBUG,
    NV_LOG_LEVEL_INFO,
    NV_LOG_LEVEL_WARN,
    NV_LOG_LEVEL_ERROR
} nv_log_level_e;

#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x

int  nv_log_get_level(void);
void nv_log_set_level(nv_log_level_e level);
char *nv_log_get_info(nv_log_level_e level);

void nv_log_set_console(int enable);
int  nv_log_get_console(void);

int  nv_log_init(void);
int  nv_log_init_file(const char *path);
int  nv_log_init_syslog(const char *ident);
void nv_log_emit(nv_log_level_e level, const char *file, int line,
                 const char *format, ...);
void nv_log_write(const char *format, ...);
void nv_log_close(void);

void printf_hex(const uint8_t *data, size_t len);

extern char log_prex[];

#define nv_log_dump(buf, len, format, args...) \
{ \
    uint8_t *temp = (uint8_t *)(buf); \
    nv_log_emit(NV_LOG_LEVEL_DUMP, __FILE__, __LINE__, format, ##args); \
    if (nv_log_get_level() <= NV_LOG_LEVEL_DUMP) \
    { \
        for (int i = 0; i < (int)(len); i++) \
        { \
            if (((i % 16) == 0)) \
                fprintf(stderr, "\n  "); \
            fprintf(stderr, "%02X ", temp[i]); \
        } \
        fprintf(stderr, "\n\n"); \
    } \
}

#define nv_log_debug(format, args...) \
    nv_log_emit(NV_LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##args)

#define nv_log_info(format, args...) \
    nv_log_emit(NV_LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##args)

#define nv_log_warning(format, args...) \
    nv_log_emit(NV_LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##args)

#define nv_log_error(format, args...) \
    nv_log_emit(NV_LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##args)

#define nv_assert(cond, ret) \
{ \
    if (!(cond)) \
    { \
        nv_log_error("!nv Assert, ret = -0x%X, %s:%d  (%s)\n", -ret, __FILE__, __LINE__, __func__); \
        return ret; \
    } \
}

#define nv_assert_pointer(cond, ret) \
{ \
    if (!(cond)) \
    { \
        nv_log_error("!nv Assert, ret = -0x%X, %s:%d  (%s)\n", -ret, __FILE__, __LINE__, __func__); \
        return NULL; \
    } \
}

#ifdef __cplusplus
}
#endif

#endif
