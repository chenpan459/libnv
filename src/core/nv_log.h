
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) nvlog, Inc.
 */


#ifndef _NV_LOG_H_INCLUDED_
#define _NV_LOG_H_INCLUDED_


#include <nv_config.h>
#include <nv_core.h>



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

// 定义日志级别
typedef enum {
    NV_LOG_DEBUG = 0,
    NV_LOG_INFO,
    NV_LOG_WARN,
    NV_LOG_ERROR,
    NV_LOG_FATAL
} NV_LogLevel;

// 日志级别对应的字符串
static const char* level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

// 定义日志颜色（ANSI转义序列）
static const char* level_colors[] = {
    "\x1b[36m",  // DEBUG - 青色
    "\x1b[32m",  // INFO - 绿色
    "\x1b[33m",  // WARN - 黄色
    "\x1b[31m",  // ERROR - 红色
    "\x1b[35m"   // FATAL - 紫色
};

// 重置颜色的ANSI转义序列
#define COLOR_RESET "\x1b[0m"


// 核心日志打印函数
void nv_log_print(NV_LogLevel level, const char* file, int line, const char* func, const char* format, ...) ;

// 便捷的日志宏定义
#define nv_log_debug(format, ...) log_print(LOG_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_info(format, ...)  log_print(LOG_INFO,  __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_warn(format, ...)  log_print(LOG_WARN,  __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_error(format, ...) log_print(LOG_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define nv_log_fatal(format, ...) log_print(LOG_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)



#endif /* _NGX_LOG_H_INCLUDED_ */
