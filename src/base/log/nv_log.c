#include "nv_log.h"


static char *log_info[] = 
{
    "Dump",
    "Debug",
    "Info",
    "Warn",
    "Error",
};

char log_prex[128];
static int nv_log_level = NV_LOG_LEVEL_DUMP;//NV_LOG_LEVEL_ERROR;

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


static FILE *log_file = NULL;
// 初始化日志文件
int nv_log_init() {
     // 获取当前时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[64]={0};
    char filename[128]={0};

    // 写入时间戳
    sprintf(time_str,"%04d%02d%02d-%02d%02d%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
            
    sprintf(filename, "/tmp/nv%slog",time_str);
        
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        return -1;
    }
    return 0;
}

// 写日志
void nv_log_write(const char *format, ...) {
    if (log_file == NULL) {
        if(nv_log_init()){
             return ;
        }
    }

    // 写入日志消息
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    //fprintf(log_file, "\n");
    fflush(log_file);
}

// 关闭日志文件
void nv_log_close(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}