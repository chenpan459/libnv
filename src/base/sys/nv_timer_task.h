

#ifndef _NV_TIMER_TASK_H_INCLUDED_
#define _NV_TIMER_TASK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"

#include <time.h>
#include <sys/timerfd.h>
#include <poll.h>

// 定义定时器回调函数类型
typedef void (*nv_timer_callback_t)(void*);

// 定义定时器结构
typedef struct {
    int fd;                     // 定时器文件描述符
    nv_timer_callback_t callback;  // 定时器超时时执行的回调函数
    void* user_data;            // 用户数据，将传递给回调函数
} nv_timer_t;

// 定义定时器管理器结构
typedef struct {
    nv_timer_t* timers;           // 定时器数组
    size_t capacity;            // 定时器数组的容量
    size_t size;                // 定时器数组当前的大小
} nv_timer_manager_t;

nv_timer_manager_t* nv_timer_manager_init(size_t capacity) ;
int nv_timer_manager_create_timer(nv_timer_manager_t* manager, time_t seconds, long nanoseconds, nv_timer_callback_t callback, void* user_data) ;
void nv_timer_manager_destroy(nv_timer_manager_t* manager) ;
// 处理定时器超时
void nv_timer_manager_process_timers(nv_timer_manager_t* manager) ;

    


int nv_timer_task_main() ;

#ifdef __cplusplus
}
#endif


#endif