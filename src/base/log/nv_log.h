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

int nv_log_get_level(void);
void nv_log_set_level(nv_log_level_e level);
char *nv_log_get_info(nv_log_level_e level);

// 初始化日志文件
int nv_log_init(void);
// 写日志
void nv_log_write( const char *format, ...);
// 关闭日志文件
void nv_log_close(void);

extern char log_prex[];
#define HEADER_FORMAT   "%-30s ] : "
#define nv_log_printf(level, file, line, format, args...) \
{ \
    if (nv_log_get_level() <= level) \
    { \
        char time_str[64]; \
        time_t now = time(NULL); \
        struct tm *t = localtime(&now); \
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t); \
        sprintf(log_prex, "[ %s %s:%d#%s", nv_log_get_info(level), filename(file), line,time_str);\
        printf(HEADER_FORMAT, log_prex); \
        printf(format, ##args); \
        nv_log_write(HEADER_FORMAT, log_prex); \
        nv_log_write(format, ##args); \
    } \
}

#define nv_log_dump(buf, len, format, args...) \
{ \
    uint8_t *temp = (uint8_t *)(buf); \
    nv_log_printf(NV_LOG_LEVEL_DUMP, __FILE__, __LINE__, format, ##args); \
    if(nv_log_get_level() <= nv_LOG_LEVEL_DUMP) \
    { \
        for (int i = 0; i < len; i++) \
        { \
            if (((i % 16) == 0)) \
                printf("\n  "); \
            printf("%02X ", temp[i]); \
        } \
        printf("\n\n"); \
    } \
}

#define nv_log_debug(format, args...) \
        nv_log_printf(NV_LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##args);

#define nv_log_info(format, args...) \
        nv_log_printf(NV_LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##args);

#define nv_log_warning(format, args...) \
        nv_log_printf(NV_LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##args);

#define nv_log_error(format, args...) \
        nv_log_printf(NV_LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##args);

#define nv_assert(cond, ret) \
{ \
    if (!(cond)) \
    { \
        nv_log_error("!nv Assert, ret = -0x%X, %s:%d  (%s)\n", -ret, __FILE__, __LINE__, __func__); \
        return ret; \
    } \
}


#ifdef __cplusplus
}
#endif

#endif