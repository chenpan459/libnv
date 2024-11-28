

#ifndef _NV_SYS_H_INCLUDED_
#define _NV_SYS_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>
#include <unistd.h> // 包含 sleep 函数的头文件
#include <time.h> // 包含 nanosleep 函数的头文件


// 声明 nv_sleep 函数
void nv_sleep(unsigned int seconds);

// milliseconds 单位毫秒
void nv_msleep(unsigned int milliseconds);

// nanoseconds 单位纳秒
void nv_nsleep(unsigned int nanoseconds);


#ifdef __cplusplus
}
#endif

#endif