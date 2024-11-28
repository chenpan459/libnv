
#ifndef _NV_TIME_H_INCLUDED_
#define _NV_TIME_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include "nv_cmd.h"
#include <strings.h> // 包含 strncasecmp 的头文件

typedef struct datetime_s {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int ms;
} datetime_t;

int time_diff(char *time1_str,char *time2_str);
int get_current_time(char * cur_time) ;

void convert_timestamp_to_datetime(const char* timestamp_str,char *timeBuff,int timeBufSize) ;
time_t datetime_to_timestamp(const char *datetime_str) ;
time_t nv_time_now();
// 获取当前时间戳(毫秒)
long long nv_time_now_ms() ;
void nv_sleep_ms(int milliseconds) ;
void nv_time_format(time_t timestamp, char* buf, int len, const char* format) ;

// 设置系统时间的函数
int nv_set_system_time(const char *time_str);

int month_atoi(const char* month) ;
const char* month_itoa(int month) ;
datetime_t nv_compile_datetime() ;



int nv_time_main();
#ifdef __cplusplus
}
#endif


#endif