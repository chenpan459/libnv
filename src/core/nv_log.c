

/************************************************
 * @文件名: nv_log.c
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 ***********************************************/

#include <nv_config.h>
#include <nv_core.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#include <nv_log.h>




// 日志级别对应的字符串
static const char* nv_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

// 定义日志颜色（ANSI转义序列）
static const char* nv_level_colors[] = {
    "\x1b[36m",  // DEBUG - 青色
    "\x1b[37m",   // WHITE - 白色//"\x1b[32m",  // INFO - 绿色
    "\x1b[33m",  // WARN - 黄色
    "\x1b[31m",  // ERROR - 红色
    "\x1b[35m"   // FATAL - 紫色
};

// 重置颜色的ANSI转义序列
#define NV_COLOR_RESET "\x1b[0m"

// 内部变量
static NV_LogLevel g_nv_log_level = NV_LOG_DEBUG;  // 当前日志级别
static FILE* g_nv_log_file = NULL;                 // 日志文件指针
static pthread_mutex_t g_nv_log_mutex = PTHREAD_MUTEX_INITIALIZER;  // 互斥锁

// 获取当前时间字符串
static void nv_get_time_str(char* buffer, size_t size) {
    time_t now;
    struct tm* timeinfo;
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

// 获取线程ID
static pthread_t nv_get_thread_id(void) {
    return pthread_self();
}

// 初始化日志系统
int nv_log_init(const char* filename) {
    pthread_mutex_lock(&g_nv_log_mutex);
    
    // 如果已经打开，先关闭
    if (g_nv_log_file != NULL) {
        fclose(g_nv_log_file);
    }
    
    // 打开日志文件
    g_nv_log_file = fopen(filename, "a");
    if (g_nv_log_file == NULL) {
        pthread_mutex_unlock(&g_nv_log_mutex);
        return -1;
    }
    
    // 写入启动日志
    char time_str[32];
    nv_get_time_str(time_str, sizeof(time_str));
    fprintf(g_nv_log_file, "\n[%s][INFO] ====== NV Log system initialized ======\n", time_str);
    fflush(g_nv_log_file);
    
    pthread_mutex_unlock(&g_nv_log_mutex);
    return 0;
}

// 关闭日志系统
void nv_log_close(void) {
    pthread_mutex_lock(&g_nv_log_mutex);
    
    if (g_nv_log_file != NULL) {
        char time_str[32];
        nv_get_time_str(time_str, sizeof(time_str));
        fprintf(g_nv_log_file, "[%s][INFO] ====== NV Log system closed ======\n", time_str);
        fflush(g_nv_log_file);
        fclose(g_nv_log_file);
        g_nv_log_file = NULL;
    }
    
    pthread_mutex_unlock(&g_nv_log_mutex);
}

// 设置日志级别
void nv_log_set_level(NV_LogLevel level) {
    pthread_mutex_lock(&g_nv_log_mutex);
    g_nv_log_level = level;
    pthread_mutex_unlock(&g_nv_log_mutex);
}

// 核心日志打印函数
void nv_log_print(NV_LogLevel level, const char* file, int line, const char* func, const char* format, ...) {
    if (level < g_nv_log_level) return;
    
    pthread_mutex_lock(&g_nv_log_mutex);
    
    // 获取时间
    char time_str[32];
    nv_get_time_str(time_str, sizeof(time_str));
    
    // 获取线程ID
    pthread_t tid = nv_get_thread_id();
    
    // 提取文件名（去掉路径）
    const char* filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    // 打印到控制台
    printf("%s[%s][%s][%lu][%s:%d][%s] ", 
           nv_level_colors[level],
           time_str,
           nv_level_strings[level],
           (unsigned long)tid,
           filename,
           line,
           func);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("%s\n", NV_COLOR_RESET);
    
    // 写入日志文件
    if (g_nv_log_file != NULL) {
        fprintf(g_nv_log_file, "[%s][%s][%lu][%s:%d][%s] ",
                time_str,
                nv_level_strings[level],
                (unsigned long)tid,
                filename,
                line,
                func);
        
        va_start(args, format);
        vfprintf(g_nv_log_file, format, args);
        va_end(args);
        
        fprintf(g_nv_log_file, "\n");
        fflush(g_nv_log_file);
    }
    
    pthread_mutex_unlock(&g_nv_log_mutex);
}





