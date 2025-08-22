/************************************************
 * @文件名: nv_event.c
 * @功能: libnv事件层实现文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，实现事件处理接口
 ***********************************************/

#include "nv_event.h"
#include <string.h>
#include <errno.h>

/* 事件初始化 */
int nv_event_init(nv_event_ext_t *ev, nv_event_handler_t handler, void *data) {
    if (!ev || !handler) {
        errno = EINVAL;
        return -1;
    }
    
    /* 清零事件结构 */
    memset(ev, 0, sizeof(nv_event_ext_t));
    
    /* 设置基本属性 */
    ev->handler = handler;
    ev->data = data;
    ev->fd = -1;
    ev->events = 0;
    ev->revents = 0;
    ev->active = 0;
    ev->type = NV_EVENT_TYPE_IO;
    ev->priority = NV_EVENT_PRIO_NORMAL;
    ev->timeout_ms = 0;
    ev->expire_time = 0;
    ev->next = NULL;
    ev->prev = NULL;
    ev->loop = NULL;
    ev->trigger_count = 0;
    ev->last_trigger = 0;
    
    return 0;
}

/* 事件清理 */
void nv_event_cleanup(nv_event_ext_t *ev) {
    if (!ev) return;
    
    /* 从事件循环中移除 */
    if (ev->loop) {
        /* 这里应该调用事件循环的移除函数 */
        ev->loop = NULL;
    }
    
    /* 从链表中移除 */
    if (ev->next) {
        ev->next->prev = ev->prev;
    }
    if (ev->prev) {
        ev->prev->next = ev->next;
    }
    ev->next = NULL;
    ev->prev = NULL;
    
    /* 重置事件状态 */
    ev->fd = -1;
    ev->events = 0;
    ev->revents = 0;
    ev->active = 0;
    ev->handler = NULL;
    ev->data = NULL;
}

/* 设置文件描述符 */
int nv_event_set_fd(nv_event_ext_t *ev, int fd) {
    if (!ev) {
        errno = EINVAL;
        return -1;
    }
    
    ev->fd = fd;
    return 0;
}

/* 设置监听事件 */
int nv_event_set_events(nv_event_ext_t *ev, int events) {
    if (!ev) {
        errno = EINVAL;
        return -1;
    }
    
    ev->events = events;
    return 0;
}

/* 设置事件优先级 */
int nv_event_set_priority(nv_event_ext_t *ev, nv_event_priority_t priority) {
    if (!ev || priority >= NV_EVENT_PRIO_URGENT) {
        errno = EINVAL;
        return -1;
    }
    
    ev->priority = priority;
    return 0;
}

/* 设置超时时间 */
int nv_event_set_timeout(nv_event_ext_t *ev, unsigned long timeout_ms) {
    if (!ev) {
        errno = EINVAL;
        return -1;
    }
    
    ev->timeout_ms = timeout_ms;
    if (timeout_ms > 0) {
        ev->type = NV_EVENT_TYPE_TIMER;
        /* 计算过期时间（当前时间 + 超时时间） */
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ev->expire_time = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000) + timeout_ms;
    }
    
    return 0;
}

/* 检查事件是否激活 */
int nv_event_is_active(nv_event_ext_t *ev) {
    return ev ? ev->active : 0;
}

/* 检查事件是否待处理 */
int nv_event_is_pending(nv_event_ext_t *ev) {
    return ev ? (ev->active && ev->revents) : 0;
}

/* 获取文件描述符 */
int nv_event_get_fd(nv_event_ext_t *ev) {
    return ev ? ev->fd : -1;
}

/* 获取监听事件 */
int nv_event_get_events(nv_event_ext_t *ev) {
    return ev ? ev->events : 0;
}

/* 事件触发处理 */
void nv_event_trigger(nv_event_ext_t *ev, int revents) {
    if (!ev || !ev->handler) return;
    
    /* 更新事件状态 */
    ev->revents = revents;
    ev->trigger_count++;
    
    /* 更新最后触发时间 */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ev->last_trigger = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    
    /* 调用事件处理函数 */
    if (ev->loop) {
        ev->handler(ev->loop, ev, ev->data);
    }
}

/* 检查定时器是否过期 */
int nv_event_is_expired(nv_event_ext_t *ev) {
    if (!ev || ev->type != NV_EVENT_TYPE_TIMER) return 0;
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    
    return now >= ev->expire_time;
}

/* 获取剩余时间 */
unsigned long nv_event_get_remaining_time(nv_event_ext_t *ev) {
    if (!ev || ev->type != NV_EVENT_TYPE_TIMER) return 0;
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    
    if (now >= ev->expire_time) return 0;
    return ev->expire_time - now;
}

