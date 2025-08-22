/************************************************
 * @文件名: nv_event.h
 * @功能: libnv事件层头文件，定义事件处理接口
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义事件处理接口
 * 2024-11-04 - 完善事件层架构设计
 ***********************************************/

#ifndef _NV_EVENT_H_INCLUDED_
#define _NV_EVENT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_core.h>

/* 前向声明 */
typedef struct nv_loop_s nv_loop_t;

/* 事件类型定义 */
#define NV_EVENT_READ     0x0001
#define NV_EVENT_WRITE    0x0002
#define NV_EVENT_ERROR    0x0004
#define NV_EVENT_HUP      0x0008
#define NV_EVENT_TIMER    0x0010
#define NV_EVENT_SIGNAL   0x0020
#define NV_EVENT_IDLE     0x0040

/* 事件状态 */
#define NV_EVENT_ACTIVE   0x0001
#define NV_EVENT_TIMEOUT  0x0002
#define NV_EVENT_POSTED   0x0004
#define NV_EVENT_PENDING  0x0008

/* 事件处理函数类型 */
typedef void (*nv_event_handler_t)(nv_loop_t *loop, void *ev, void *data);

/* 扩展事件结构（如果核心层没有定义的话） */
#ifndef NV_EVENT_STRUCT_DEFINED
#define NV_EVENT_STRUCT_DEFINED
typedef struct nv_event_ext_s {
    /* 事件标识 */
    int fd;                     /* 文件描述符，-1表示非IO事件 */
    int events;                 /* 监听的事件类型 */
    int revents;                /* 触发的事件类型 */
    int active;                 /* 事件是否激活 */
    
    /* 事件处理 */
    nv_event_handler_t handler; /* 事件处理函数 */
    void *data;                 /* 用户数据 */
    
    /* 定时器相关 */
    unsigned long timeout_ms;   /* 超时时间(毫秒) */
    unsigned long expire_time;  /* 过期时间 */
    
    /* 链表管理 */
    struct nv_event_ext_s *next;    /* 下一个事件 */
    struct nv_event_ext_s *prev;    /* 上一个事件 */
    
    /* 事件循环引用 */
    nv_loop_t *loop;            /* 所属的事件循环 */
    
    /* 事件类型 */
    unsigned char type;         /* 事件类型：IO/TIMER/SIGNAL/IDLE */
    unsigned char priority;     /* 事件优先级 */
    
    /* 统计信息 */
    unsigned long trigger_count; /* 触发次数 */
    unsigned long last_trigger;  /* 最后触发时间 */
} nv_event_ext_t;
#endif

/* 事件类型枚举 */
typedef enum {
    NV_EVENT_TYPE_IO = 0,      /* IO事件 */
    NV_EVENT_TYPE_TIMER,       /* 定时器事件 */
    NV_EVENT_TYPE_SIGNAL,      /* 信号事件 */
    NV_EVENT_TYPE_IDLE         /* 空闲事件 */
} nv_event_type_t;

/* 事件优先级 */
typedef enum {
    NV_EVENT_PRIO_LOW = 0,     /* 低优先级 */
    NV_EVENT_PRIO_NORMAL,      /* 普通优先级 */
    NV_EVENT_PRIO_HIGH,        /* 高优先级 */
    NV_EVENT_PRIO_URGENT       /* 紧急优先级 */
} nv_event_priority_t;

/* 事件API函数声明 */
int nv_event_init(nv_event_ext_t *ev, nv_event_handler_t handler, void *data);
void nv_event_cleanup(nv_event_ext_t *ev);

/* 事件属性设置 */
int nv_event_set_fd(nv_event_ext_t *ev, int fd);
int nv_event_set_events(nv_event_ext_t *ev, int events);
int nv_event_set_priority(nv_event_ext_t *ev, nv_event_priority_t priority);
int nv_event_set_timeout(nv_event_ext_t *ev, unsigned long timeout_ms);

/* 事件状态查询 */
int nv_event_is_active(nv_event_ext_t *ev);
int nv_event_is_pending(nv_event_ext_t *ev);
int nv_event_get_fd(nv_event_ext_t *ev);
int nv_event_get_events(nv_event_ext_t *ev);

/* 事件触发处理 */
void nv_event_trigger(nv_event_ext_t *ev, int revents);

/* 事件初始化宏 */
#define NV_EVENT_INIT(ev, handler, data) do { \
    memset((ev), 0, sizeof(*(ev))); \
    (ev)->handler = (handler); \
    (ev)->data = (data); \
    (ev)->fd = -1; \
    (ev)->events = 0; \
    (ev)->revents = 0; \
    (ev)->active = 0; \
    (ev)->type = NV_EVENT_TYPE_IO; \
    (ev)->priority = NV_EVENT_PRIO_NORMAL; \
} while(0)

/* 事件类型设置宏 */
#define NV_EVENT_SET_TYPE(ev, type) ((ev)->type = (type))
#define NV_EVENT_SET_PRIORITY(ev, prio) ((ev)->priority = (prio))

#ifdef __cplusplus
}
#endif

#endif /* _NV_EVENT_H_INCLUDED_ */

