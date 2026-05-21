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

/**
 * 队列满时的策略（业务线程均为无锁入队，永不等待 IO/互斥锁）
 * BLOCK 与 DROP 相同：丢弃新日志（不在业务线程阻塞）
 */
typedef enum {
    NV_LOG_OVERFLOW_DROP = 0,
    NV_LOG_OVERFLOW_BLOCK,         /**< 同 DROP，业务线程不阻塞 */
    NV_LOG_OVERFLOW_OVERWRITE,     /**< 无锁丢弃最旧一条后写入 */
} nv_log_overflow_e;

#define filename(x) (strrchr(x, '/') ? strrchr(x, '/') + 1 : x)

#define NV_LOG_LEVEL_WARN       NV_LOG_LEVEL_WARNING
#define NV_LOG_LEVEL_DUMP       NV_LOG_LEVEL_DEBUG
#define NV_LOG_LINE_MAX         4096
#define NV_LOG_QUEUE_DEFAULT    2048
#define NV_LOG_QUEUE_MIN        64
#define NV_LOG_QUEUE_MAX        65536

int  nv_log_get_level(void);
void nv_log_set_level(nv_log_level_e level);
const char *nv_log_get_info(nv_log_level_e level);
int  nv_log_level_from_string(const char *name, nv_log_level_e *out);
int  nv_log_overflow_from_string(const char *name, nv_log_overflow_e *out);

void nv_log_set_module(const char *module);
const char *nv_log_get_module(void);

void nv_log_set_console(int enable);
int  nv_log_get_console(void);

/** 须在 nv_log_init / nv_log_init_file 之前调用 */
void nv_log_set_queue_size(size_t capacity);
size_t nv_log_get_queue_size(void);

void nv_log_set_overflow_policy(nv_log_overflow_e policy);
nv_log_overflow_e nv_log_get_overflow_policy(void);
uint64_t nv_log_get_dropped_count(void);

typedef struct nv_log_queue_stats_s {
    size_t   capacity;
    size_t   pending;
    uint64_t dropped;
    const char *overflow;   /* drop|block|overwrite */
} nv_log_queue_stats_t;

void nv_log_get_queue_stats(nv_log_queue_stats_t *stats);

/** 日志文件按大小轮转（须在 nv_log_init_file 之前或之后均可，写盘前检查） */
void nv_log_set_rotate(size_t max_bytes, int keep_files);

int  nv_log_init(void);
int  nv_log_init_file(const char *path);
int  nv_log_init_syslog(const char *ident);

/**
 * 输出格式:
 * [时间戳] [级别] [PID:x TID:y] [模块] 文件:行号 内容
 *
 * 业务线程：栈上格式化 + 无锁 MPSC 环形缓冲区入队，立即返回。
 * 日志线程：条件变量休眠，单线程写 stderr / 文件 / syslog。
 */
void nv_log_emit(nv_log_level_e level, const char *module,
                 const char *file, int line, const char *format, ...);

void nv_log_write(const char *format, ...);
void nv_log_close(void);

void nv_log_hex_dump(const uint8_t *data, size_t len);
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

#define nv_log_info_m(mod, format, args...) \
    nv_log_emit(NV_LOG_LEVEL_INFO, mod, __FILE__, __LINE__, format, ##args)

#define nv_log_dump(buf, len, format, args...) \
    do { \
        nv_log_emit_lv(NV_LOG_LEVEL_DEBUG, format, ##args); \
        if (nv_log_get_level() <= NV_LOG_LEVEL_DEBUG) { \
            nv_log_hex_dump((const uint8_t *)(buf), (size_t)(len)); \
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
