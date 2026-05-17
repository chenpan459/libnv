#ifndef _NV_LOG_H_INCLUDED_
#define _NV_LOG_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

/**
 * 日志等级（数值越小越详细，过滤时仅输出 >= 设定等级的日志）
 */
typedef enum {
    NV_LOG_LEVEL_DEBUG = 0,
    NV_LOG_LEVEL_INFO,
    NV_LOG_LEVEL_NOTICE,
    NV_LOG_LEVEL_WARNING,
    NV_LOG_LEVEL_ERROR,
    NV_LOG_LEVEL_CRIT,
    NV_LOG_LEVEL_FATAL,
    NV_LOG_LEVEL_MAX
} nv_log_level_e;

#define filename(x) (strrchr(x, '/') ? strrchr(x, '/') + 1 : x)

#define NV_LOG_LEVEL_WARN  NV_LOG_LEVEL_WARNING
#define NV_LOG_LEVEL_DUMP  NV_LOG_LEVEL_DEBUG

int  nv_log_get_level(void);
void nv_log_set_level(nv_log_level_e level);
const char *nv_log_get_info(nv_log_level_e level);
int  nv_log_level_from_string(const char *name, nv_log_level_e *out);

void nv_log_set_module(const char *module);
const char *nv_log_get_module(void);

void nv_log_set_console(int enable);
int  nv_log_get_console(void);

int  nv_log_init(void);
int  nv_log_init_file(const char *path);
int  nv_log_init_syslog(const char *ident);

/**
 * 输出格式:
 * [时间戳] [级别] [PID:x TID:y] [模块] 文件:行号 内容
 */
void nv_log_emit(nv_log_level_e level, const char *module,
                 const char *file, int line, const char *format, ...);

void nv_log_write(const char *format, ...);
void nv_log_close(void);

void printf_hex(const uint8_t *data, size_t len);

#define nv_log_emit_lv(level, format, args...) \
    nv_log_emit(level, NULL, __FILE__, __LINE__, format, ##args)

#define nv_log_debug(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_DEBUG, format, ##args)

#define nv_log_info(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_INFO, format, ##args)

#define nv_log_notice(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_NOTICE, format, ##args)

#define nv_log_warning(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_WARNING, format, ##args)

#define nv_log_warn  nv_log_warning

#define nv_log_error(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_ERROR, format, ##args)

#define nv_log_crit(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_CRIT, format, ##args)

#define nv_log_fatal(format, args...) \
    nv_log_emit_lv(NV_LOG_LEVEL_FATAL, format, ##args)

/* 指定模块名（覆盖全局默认模块） */
#define nv_log_info_m(mod, format, args...) \
    nv_log_emit(NV_LOG_LEVEL_INFO, mod, __FILE__, __LINE__, format, ##args)

#define nv_log_dump(buf, len, format, args...) \
    do { \
        nv_log_emit_lv(NV_LOG_LEVEL_DEBUG, format, ##args); \
        if (nv_log_get_level() <= NV_LOG_LEVEL_DEBUG) { \
            const uint8_t *_nv_dump_p = (const uint8_t *)(buf); \
            int _nv_dump_i; \
            for (_nv_dump_i = 0; _nv_dump_i < (int)(len); _nv_dump_i++) { \
                if ((_nv_dump_i % 16) == 0) { \
                    fprintf(stderr, "\n  "); \
                } \
                fprintf(stderr, "%02X ", _nv_dump_p[_nv_dump_i]); \
            } \
            fprintf(stderr, "\n\n"); \
        } \
    } while (0)

#define nv_assert(cond, ret) \
    do { \
        if (!(cond)) { \
            nv_log_error("!nv Assert, ret = -0x%X, %s:%d (%s)\n", \
                         -(ret), __FILE__, __LINE__, __func__); \
            return (ret); \
        } \
    } while (0)

#define nv_assert_pointer(cond, ret) \
    do { \
        if (!(cond)) { \
            nv_log_error("!nv Assert, ret = -0x%X, %s:%d (%s)\n", \
                         -(ret), __FILE__, __LINE__, __func__); \
            return NULL; \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
