
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <nv_log.h>
#include <nv_config.h>
#include <nv_core.h>


// 当前日志级别（可以通过函数修改）
static NV_LogLevel current_log_level = NV_LOG_DEBUG;

// 日志文件指针
static FILE* log_file = NULL;

// 设置日志级别
void nv_set_log_level(NV_LogLevel level) {
    current_log_level = level;
}

// 初始化日志系统
int nv_init_logger(const char* filename) {
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        printf("无法打开日志文件: %s\n", filename);
        return -1;
    }
    return 0;
}

// 关闭日志系统
void nv_close_logger() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

// 核心日志打印函数
void nv_log_print(NV_LogLevel level, const char* file, int line, const char* func, const char* format, ...) {
    if (level < current_log_level) return;

    time_t now;
    time(&now);
    struct tm* local_time = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    // 打印到控制台
    printf("%s[%s][%s][%s:%d][%s] ", 
           level_colors[level],
           time_str,
           level_strings[level],
           file,
           line,
           func);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\n", COLOR_RESET);

    // 写入日志文件
    if (log_file != NULL) {
        fprintf(log_file, "[%s][%s][%s:%d][%s] ",
                time_str,
                level_strings[level],
                file,
                line,
                func);

        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);

        fprintf(log_file, "\n");
        fflush(log_file);
    }
}




