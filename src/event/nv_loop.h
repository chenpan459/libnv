/************************************************
 * @文件名: nv_loop.h
 * @功能: libnv事件循环层头文件，定义事件循环接口
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义事件循环接口
 * 2024-11-04 - 完善事件循环层架构设计
 ***********************************************/

#ifndef _NV_LOOP_H_INCLUDED_
#define _NV_LOOP_H_INCLUDED_

#include <nv_config.h>
#include <nv_core.h>

/* 前向声明 */
typedef struct nv_event_ext_s nv_event_ext_t;
typedef struct nv_loop_stats_s nv_loop_stats_t;

/* 事件循环状态 */
typedef enum {
    NV_LOOP_INIT = 0,
    NV_LOOP_RUNNING,
    NV_LOOP_STOPPING,
    NV_LOOP_STOPPED
} nv_loop_state_t;

/* 事件循环配置 */
typedef struct nv_loop_config_s {
    int max_events;
    int timeout_ms;
    int max_fd;
    int use_epoll;
    int use_poll;
    int use_select;
    int enable_timers;
    int enable_signals;
    int enable_idle;
} nv_loop_config_t;

/* 事件循环结构 */
typedef struct nv_loop_s {
    int epoll_fd;
    int max_events;
    struct epoll_event *events;
    int event_count;
    nv_loop_state_t state;
    nv_pool_t *pool;
    
    /* 事件管理 */
    nv_event_ext_t *timer_events;
    nv_event_ext_t *signal_events;
    nv_event_ext_t *idle_events;
    
    /* 定时器管理 */
    struct {
        nv_event_ext_t *head;
        nv_event_ext_t *tail;
        unsigned long next_timeout;
    } timers;
    
    /* 信号管理 */
    struct {
        int *signals;
        int signal_count;
        nv_event_ext_t **signal_events;
    } signals;
    
    /* 空闲事件管理 */
    struct {
        nv_event_ext_t *head;
        nv_event_ext_t *tail;
    } idle;
    
    /* 统计信息 */
    struct {
        unsigned long events_processed;
        unsigned long timers_processed;
        unsigned long signals_processed;
        unsigned long idle_events_processed;
    } stats;
    
    void *private_data;
} nv_loop_t;

/* 事件循环统计结构 */
typedef struct nv_loop_stats_s {
    unsigned long events_processed;
    unsigned long timers_processed;
    unsigned long signals_processed;
    unsigned long idle_events_processed;
    unsigned long total_events;
    unsigned long active_events;
    unsigned long pending_events;
} nv_loop_stats_t;

/* 事件循环API函数声明 */
int nv_loop_init(nv_loop_t *loop, const nv_loop_config_t *config);
int nv_loop_cleanup(nv_loop_t *loop);
int nv_loop_run(nv_loop_t *loop);
int nv_loop_stop(nv_loop_t *loop);
int nv_loop_wakeup(nv_loop_t *loop);

/* 事件管理API */
int nv_loop_add_event(nv_loop_t *loop, nv_event_ext_t *ev, int events);
int nv_loop_del_event(nv_loop_t *loop, nv_event_ext_t *ev);
int nv_loop_modify_event(nv_loop_t *loop, nv_event_ext_t *ev, int events);

/* 定时器管理API */
int nv_loop_add_timer(nv_loop_t *loop, nv_event_ext_t *ev, unsigned long timeout_ms);
int nv_loop_del_timer(nv_loop_t *loop, nv_event_ext_t *ev);
int nv_loop_modify_timer(nv_loop_t *loop, nv_event_ext_t *ev, unsigned long timeout_ms);

/* 信号管理API */
int nv_loop_add_signal(nv_loop_t *loop, nv_event_ext_t *ev, int signo);
int nv_loop_del_signal(nv_loop_t *loop, nv_event_ext_t *ev);
int nv_loop_ignore_signal(nv_loop_t *loop, int signo);

/* 空闲事件管理API */
int nv_loop_add_idle(nv_loop_t *loop, nv_event_ext_t *ev);
int nv_loop_del_idle(nv_loop_t *loop, nv_event_ext_t *ev);

/* 事件循环控制API */
int nv_loop_pause(nv_loop_t *loop);
int nv_loop_resume(nv_loop_t *loop);
int nv_loop_is_running(nv_loop_t *loop);

/* 事件循环统计API */
void nv_loop_get_stats(nv_loop_t *loop, nv_loop_stats_t *stats);
void nv_loop_reset_stats(nv_loop_t *loop);

/* 事件循环配置API */
int nv_loop_set_config(nv_loop_t *loop, const nv_loop_config_t *config);
int nv_loop_get_config(nv_loop_t *loop, nv_loop_config_t *config);

/* 事件循环工具函数 */
unsigned long nv_loop_now(nv_loop_t *loop);
int nv_loop_update_time(nv_loop_t *loop);

/* 事件循环配置默认值 */
#define NV_LOOP_CONFIG_DEFAULT { \
    .max_events = 1024, \
    .timeout_ms = 1000, \
    .max_fd = 65536, \
    .use_epoll = 1, \
    .use_poll = 0, \
    .use_select = 0, \
    .enable_timers = 1, \
    .enable_signals = 1, \
    .enable_idle = 1 \
}

/* 事件循环创建宏 */
#define NV_LOOP_CREATE(name, config) \
    nv_loop_t name; \
    nv_loop_config_t name##_config = config; \
    if (nv_loop_init(&name, &name##_config) != NV_OK) { \
        return NV_ERROR; \
    }

/* 事件循环销毁宏 */
#define NV_LOOP_DESTROY(name) \
    nv_loop_cleanup(&name)

#endif /* _NV_LOOP_H_INCLUDED_ */
