

#ifndef _NV_TIMER_TASK_H_INCLUDED_
#define _NV_TIMER_TASK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>

#include <time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <pthread.h>

/* 定时回调 */
typedef void (*nv_timer_callback_t)(void*);

/* 定时器类型：一次性/周期性 */
typedef enum {
    NV_TIMER_ONESHOT = 0,
    NV_TIMER_PERIODIC = 1
} nv_timer_kind_t;

/* 定时器结构 */
typedef struct {
    int                     fd;           /* timerfd */
    nv_timer_callback_t     callback;     /* 回调 */
    void*                   user_data;    /* 用户数据 */
    nv_timer_kind_t         kind;         /* 类型 */
} nv_timer_t;

/* 定时器管理器 */
typedef struct {
    int                     epoll_fd;     /* epoll 句柄 */
    nv_timer_t*             timers;       /* 动态数组 */
    size_t                  capacity;     /* 容量 */
    size_t                  size;         /* 当前数量（包含空洞） */

    pthread_mutex_t         lock;         /* 线程安全 */
    int                     running;      /* 运行标志 */
} nv_timer_manager_t;

/* 初始化与销毁 */
nv_timer_manager_t* nv_timer_manager_init(size_t capacity);
void nv_timer_manager_destroy(nv_timer_manager_t* manager);

/* 创建定时器（返回 timerfd，<=0 表示失败）
 * seconds/nanoseconds 指首次触发；
 * interval_seconds/interval_nanoseconds 指周期（0 表示一次性） */
int nv_timer_manager_create_timer(nv_timer_manager_t* manager,
                                  time_t seconds, long nanoseconds,
                                  time_t interval_seconds, long interval_nanoseconds,
                                  nv_timer_callback_t callback, void* user_data);

/* 取消并移除定时器（传入 timerfd） */
int nv_timer_manager_cancel(nv_timer_manager_t* manager, int timer_fd);

/* 修改定时器下一次触发与周期（传入 timerfd） */
int nv_timer_manager_modify(nv_timer_manager_t* manager,
                            int timer_fd,
                            time_t seconds, long nanoseconds,
                            time_t interval_seconds, long interval_nanoseconds);

/* 处理一次事件（timeout_ms < 0 表示无限等待） */
int nv_timer_manager_process_once(nv_timer_manager_t* manager, int timeout_ms);

/* 运行循环，直到调用 stop */
int nv_timer_manager_run(nv_timer_manager_t* manager);
void nv_timer_manager_stop(nv_timer_manager_t* manager);

/* 示例入口（可选） */
int nv_timer_task_main();

#ifdef __cplusplus
}
#endif

#endif