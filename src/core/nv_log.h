
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) nvlog, Inc.
 */


#ifndef _NV_LOG_H_INCLUDED_
#define _NV_LOG_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif


#include <nv_config.h>
#include <nv_core.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// 定义日志级别
typedef enum {
    NV_LOG_DEBUG = 0,
    NV_LOG_INFO,
    NV_LOG_WARN,
    NV_LOG_ERROR,
    NV_LOG_FATAL
} NV_LogLevel;

// 对外接口声明
int nv_log_init(const char* filename);
void nv_log_close(void);
void nv_log_set_level(NV_LogLevel level);
void nv_log_print(NV_LogLevel level, const char* file, int line, const char* func, const char* format, ...);


// 便捷的日志宏定义
#define nv_log_debug(format, ...) nv_log_print(NV_LOG_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_info(format, ...)  nv_log_print(NV_LOG_INFO,  __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_warn(format, ...)  nv_log_print(NV_LOG_WARN,  __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_error(format, ...) nv_log_print(NV_LOG_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_fatal(format, ...) nv_log_print(NV_LOG_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif


#endif /* _NV_LOG_H_INCLUDED_ */
