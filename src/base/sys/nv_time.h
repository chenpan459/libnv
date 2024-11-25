
#ifndef _NV_TIME_H_INCLUDED_
#define _NV_TIME_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"

int time_diff(char *time1_str,char *time2_str);
int get_current_time(char * cur_time) ;

void convert_timestamp_to_datetime(const char* timestamp_str,char *timeBuff,int timeBufSize) ;
time_t datetime_to_timestamp(const char *datetime_str) ;
time_t nv_time_now();
// 获取当前时间戳(毫秒)
long long nv_time_now_ms() ;
void nv_sleep_ms(int milliseconds) ;
void nv_time_format(time_t timestamp, char* buf, int len, const char* format) ;





int nv_time_main();
#ifdef __cplusplus
}
#endif


#endif