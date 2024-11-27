
#include "nv_time.h"


/*********************
 * 要计算两个日期时间之间的分钟差，您可以使用C语言中的time.h库。计算了您提供的两个日期时间之间的分钟差
 * ************************************/
int time_diff(char *time1_str,char *time2_str) {
    #if 0 //c99
    // 定义两个时间字符串
    //char time1_str[] = "2024-11-13 11:33:00";
    //char time2_str[] = "2024-11-13 20:33:00";

    // 定义两个时间结构体
    struct tm time1, time2;

    // 设置时间结构体为0
    memset(&time1, 0, sizeof(struct tm));
    memset(&time2, 0, sizeof(struct tm));

    // 使用strptime将字符串转换为时间结构体
    if (strptime(time1_str, "%Y-%m-%d %H:%M:%S", &time1) == NULL ||
        strptime(time2_str, "%Y-%m-%d %H:%M:%S", &time2) == NULL) {
        printf("时间格式错误\n");
        return 1;
    }

    // 将时间结构体转换为自纪元以来的秒数
    time_t time1_sec = mktime(&time1);
    time_t time2_sec = mktime(&time2);
    // 计算秒差
    double diff_sec = difftime(time2_sec, time1_sec);
    // 将秒差转换为分钟差
    int diff_min = diff_sec / 60;
    // 输出分钟差
    printf("两个时间之间的分钟差是: %d 分钟\n", diff_min);
    return diff_min;
#else    
    // 定义两个时间结构体
    struct tm time1, time2;

    // 设置时间结构体为0
    memset(&time1, 0, sizeof(struct tm));
    memset(&time2, 0, sizeof(struct tm));

    // 使用sscanf将字符串转换为时间结构体
    if (sscanf(time1_str, "%d-%d-%d %d:%d:%d", &time1.tm_year, &time1.tm_mon, &time1.tm_mday,
               &time1.tm_hour, &time1.tm_min, &time1.tm_sec) != 6 ||
        sscanf(time2_str, "%d-%d-%d %d:%d:%d", &time2.tm_year, &time2.tm_mon, &time2.tm_mday,
               &time2.tm_hour, &time2.tm_min, &time2.tm_sec) != 6) {
        printf("时间格式错误\n");
        return 1;
    }

    // 调整结构体中的年份和月份，因为tm_year是从1900年起的年数，tm_mon是从0开始的月份
    time1.tm_year -= 1900;
    time1.tm_mon -= 1;
    time2.tm_year -= 1900;
    time2.tm_mon -= 1;
    // 将时间结构体转换为自纪元以来的秒数
    time_t time1_sec = mktime(&time1);
    time_t time2_sec = mktime(&time2);
    // 计算秒差
    double diff_sec = difftime(time2_sec, time1_sec);
    // 将秒差转换为分钟差
    int diff_min = diff_sec / 60;
    // 输出分钟差
    printf("两个时间之间的分钟差是: %d 分钟\n", diff_min);
    return diff_min;
#endif
    
}

/************************************
 * 获取系统当前时间
 * ************************************/
int get_current_time(char * cur_time) {

    if(cur_time==NULL){return 1;}
    // 获取当前时间
    time_t now = time(NULL);

    // 检查time函数是否成功
    if (now == -1) {
        puts("获取时间失败");
        return 1;
    }
    // 将时间转换为本地时间
    struct tm *local_time = localtime(&now);
   // 使用strftime将时间格式化为字符串
    if (strftime(cur_time, sizeof(cur_time), "%Y-%m-%d %H:%M:%S", local_time) == 0) {
        puts("格式化时间失败");
        return 1;
    }

    // 打印当前时间
    printf("当前时间是: %s", asctime(local_time));

    return 0;
}


// 将字符串时间戳转换为日期时间
void convert_timestamp_to_datetime(const char* timestamp_str,char *timeBuff,int timeBufSize) {
    // sudo timedatectl set-timezone Asia/Shanghai
    // 将字符串转换为长整型
    time_t timestamp = atol(timestamp_str);
    // 转换为本地时间
    struct tm *local_time = localtime(&timestamp);
    // 使用 strftime 格式化日期时间
   // char buffer[80];
    strftime(timeBuff, timeBufSize, "%Y-%m-%d %H:%M:%S", local_time);
    // 打印结果
   // printf("日期时间: %s\n", timeBuff);
}

// 将日期时间字符串转换为时间戳的函数
time_t datetime_to_timestamp(const char *datetime_str) {
     struct tm timeinfo = {0};    
    // 解析日期时间字符串
    sscanf(datetime_str, "%d-%d-%d %d:%d:%d", 
           &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday,
           &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
               
    // 年份从1900开始计算，月份从0开始计算
    timeinfo.tm_year -= 1900;
    timeinfo.tm_mon -= 1;
    // 使用mktime将tm结构体转换为时间戳
    return mktime(&timeinfo);
}


// 获取当前时间戳(秒)
time_t nv_time_now() {
    return time(NULL);
}

// 获取当前时间戳(毫秒)
long long nv_time_now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// 格式化时间戳为字符串
void nv_time_format(time_t timestamp, char* buf, int len, const char* format) {
    struct tm* tm_info = localtime(&timestamp);
    strftime(buf, len, format, tm_info);
}

// sleep指定毫秒数
void nv_sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}



// 设置系统时间的函数,时间格式："2023-12-31 23:59:00"
int nv_set_system_time(const char *time_str) {
    char command[256];

    // 使用 timedatectl 设置系统时间
    snprintf(command, sizeof(command), "date -s  \"%s\"", time_str);

    // 执行命令
    int ret = nv_system(command);
    if (ret != 0) {
        fprintf(stderr, "设置系统时间失败\n");
        return 1;
    }

    return 0;
}




int nv_time_main(){

    char buff[64]={0};
    convert_timestamp_to_datetime("1731568786",buff,sizeof(buff));

    printf("时间: %s\n", buff);
  // 日期时间字符串
    char datetime_str[] = "2024-1-1 10:10:10";    
    // 调用函数将日期时间字符串转换为时间戳
    time_t timestamp = datetime_to_timestamp(datetime_str);    
    if (timestamp != -1) {
        // 打印时间戳
        printf("时间戳: %ld\n", (long)timestamp);
    }
    
    return 0;
}