/************************************************
 * @文件名: nv_loop.c
 * @功能: libnv事件循环层实现文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，实现事件循环接口
 ***********************************************/

#include "nv_loop.h"
#include "nv_event.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <fcntl.h>

/* 内部函数声明 */
static int nv_loop_process_io_events(nv_loop_t *loop);
static int nv_loop_process_timers(nv_loop_t *loop);
static int nv_loop_process_signals(nv_loop_t *loop);
static int nv_loop_process_idle_events(nv_loop_t *loop);
static void nv_loop_add_event_to_list(nv_event_ext_t **head, nv_event_ext_t *ev);
static void nv_loop_remove_event_from_list(nv_event_ext_t **head, nv_event_ext_t *ev);
static unsigned long nv_loop_get_monotonic_time(void);

/* 事件循环初始化 */
int nv_loop_init(nv_loop_t *loop, const nv_loop_config_t *config) {
    if (!loop) {
        errno = EINVAL;
        return -1;
    }
    
    /* 使用默认配置 */
    nv_loop_config_t default_config = NV_LOOP_CONFIG_DEFAULT;
    if (!config) {
        config = &default_config;
    }
    
    /* 清零循环结构 */
    memset(loop, 0, sizeof(nv_loop_t));
    
    /* 设置基本属性 */
    loop->max_events = config->max_events;
    loop->state = NV_LOOP_INIT;
    
    /* 创建epoll实例 */
    loop->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (loop->epoll_fd == -1) {
        perror("NV: epoll_create1 failed");
        return -1;
    }
    
    /* 分配事件数组 */
    loop->events = (struct epoll_event *)malloc(loop->max_events * sizeof(struct epoll_event));
    if (!loop->events) {
        perror("NV: malloc events array failed");
        close(loop->epoll_fd);
        return -1;
    }
    
    /* 初始化定时器链表 */
    loop->timers.head = NULL;
    loop->timers.tail = NULL;
    loop->timers.next_timeout = 0;
    
    /* 初始化信号管理 */
    if (config->enable_signals) {
        loop->signals.signals = NULL;
        loop->signals.signal_count = 0;
        loop->signals.signal_events = NULL;
    }
    
    /* 初始化空闲事件链表 */
    if (config->enable_idle) {
        loop->idle.head = NULL;
        loop->idle.tail = NULL;
    }
    
    /* 初始化统计信息 */
    memset(&loop->stats, 0, sizeof(loop->stats));
    
    loop->state = NV_LOOP_INIT;
    return 0;
}

/* 事件循环清理 */
int nv_loop_cleanup(nv_loop_t *loop) {
    if (!loop) return 0;
    
    /* 停止事件循环 */
    if (loop->state == NV_LOOP_RUNNING) {
        nv_loop_stop(loop);
    }
    
    /* 清理所有事件 */
    nv_event_ext_t *ev, *next;
    
    /* 清理定时器事件 */
    for (ev = loop->timers.head; ev; ev = next) {
        next = ev->next;
        nv_event_cleanup(ev);
    }
    
    /* 清理空闲事件 */
    for (ev = loop->idle.head; ev; ev = next) {
        next = ev->next;
        nv_event_cleanup(ev);
    }
    
    /* 清理信号事件 */
    if (loop->signals.signal_events) {
        for (int i = 0; i < loop->signals.signal_count; i++) {
            if (loop->signals.signal_events[i]) {
                nv_event_cleanup(loop->signals.signal_events[i]);
            }
        }
        free(loop->signals.signal_events);
        free(loop->signals.signals);
    }
    
    /* 关闭epoll */
    if (loop->epoll_fd != -1) {
        close(loop->epoll_fd);
        loop->epoll_fd = -1;
    }
    
    /* 释放事件数组 */
    if (loop->events) {
        free(loop->events);
        loop->events = NULL;
    }
    
    loop->state = NV_LOOP_STOPPED;
    return 0;
}

/* 运行事件循环 */
int nv_loop_run(nv_loop_t *loop) {
    if (!loop || loop->state == NV_LOOP_RUNNING) {
        errno = EINVAL;
        return -1;
    }
    
    loop->state = NV_LOOP_RUNNING;
    
    while (loop->state == NV_LOOP_RUNNING) {
        int timeout = -1; /* 默认阻塞 */
        
        /* 计算下次超时时间 */
        if (loop->timers.head) {
            unsigned long now = nv_loop_get_monotonic_time();
            if (loop->timers.next_timeout > now) {
                timeout = (int)(loop->timers.next_timeout - now);
            } else {
                timeout = 0; /* 立即处理定时器 */
            }
        }
        
        /* 等待事件 */
        int nfds = epoll_wait(loop->epoll_fd, loop->events, loop->max_events, timeout);
        
        if (nfds == -1) {
            if (errno == EINTR) {
                continue; /* 被信号中断，继续循环 */
            }
            perror("NV: epoll_wait failed");
            break;
        }
        
        /* 处理IO事件 */
        if (nfds > 0) {
            loop->event_count = nfds;
            nv_loop_process_io_events(loop);
        }
        
        /* 处理定时器事件 */
        nv_loop_process_timers(loop);
        
        /* 处理信号事件 */
        nv_loop_process_signals(loop);
        
        /* 处理空闲事件 */
        nv_loop_process_idle_events(loop);
    }
    
    return 0;
}

/* 停止事件循环 */
int nv_loop_stop(nv_loop_t *loop) {
    if (!loop) return -1;
    
    if (loop->state == NV_LOOP_RUNNING) {
        loop->state = NV_LOOP_STOPPING;
    }
    
    return 0;
}

/* 唤醒事件循环 */
int nv_loop_wakeup(nv_loop_t *loop) {
    if (!loop || loop->epoll_fd == -1) return -1;
    
    /* 通过写入管道来唤醒epoll_wait */
    static int wakeup_pipe[2] = {-1, -1};
    static int wakeup_initialized = 0;
    
    if (!wakeup_initialized) {
        if (pipe(wakeup_pipe) == 0) {
            fcntl(wakeup_pipe[0], F_SETFL, O_NONBLOCK);
            fcntl(wakeup_pipe[1], F_SETFL, O_NONBLOCK);
            
            /* 将读端添加到epoll */
            struct epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.ptr = NULL; /* 特殊标记 */
            epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, wakeup_pipe[0], &ev);
            
            wakeup_initialized = 1;
        }
    }
    
    if (wakeup_initialized) {
        char c = 'w';
        write(wakeup_pipe[1], &c, 1);
    }
    
    return 0;
}

/* 添加IO事件 */
int nv_loop_add_event(nv_loop_t *loop, nv_event_ext_t *ev, int events) {
    if (!loop || !ev || loop->epoll_fd == -1) {
        errno = EINVAL;
        return -1;
    }
    
    /* 设置事件属性 */
    ev->loop = loop;
    ev->events = events;
    ev->active = 1;
    
    /* 添加到epoll */
    struct epoll_event epoll_ev;
    epoll_ev.events = events;
    epoll_ev.data.ptr = ev;
    
    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, ev->fd, &epoll_ev) == -1) {
        perror("NV: epoll_ctl ADD failed");
        return -1;
    }
    
    return 0;
}

/* 删除IO事件 */
int nv_loop_del_event(nv_loop_t *loop, nv_event_ext_t *ev) {
    if (!loop || !ev || loop->epoll_fd == -1) return -1;
    
    if (ev->active && ev->fd != -1) {
        epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, ev->fd, NULL);
    }
    
    ev->active = 0;
    ev->loop = NULL;
    
    return 0;
}

/* 修改IO事件 */
int nv_loop_modify_event(nv_loop_t *loop, nv_event_ext_t *ev, int events) {
    if (!loop || !ev || loop->epoll_fd == -1) return -1;
    
    ev->events = events;
    
    if (ev->active && ev->fd != -1) {
        struct epoll_event epoll_ev;
        epoll_ev.events = events;
        epoll_ev.data.ptr = ev;
        
        if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_MOD, ev->fd, &epoll_ev) == -1) {
            perror("NV: epoll_ctl MOD failed");
            return -1;
        }
    }
    
    return 0;
}

/* 添加定时器 */
int nv_loop_add_timer(nv_loop_t *loop, nv_event_ext_t *ev, unsigned long timeout_ms) {
    if (!loop || !ev) return -1;
    
    /* 设置定时器属性 */
    ev->type = NV_EVENT_TYPE_TIMER;
    ev->timeout_ms = timeout_ms;
    ev->loop = loop;
    
    /* 计算过期时间 */
    ev->expire_time = nv_loop_get_monotonic_time() + timeout_ms;
    
    /* 添加到定时器链表 */
    nv_loop_add_event_to_list(&loop->timers.head, ev);
    
    /* 更新下次超时时间 */
    if (!loop->timers.next_timeout || ev->expire_time < loop->timers.next_timeout) {
        loop->timers.next_timeout = ev->expire_time;
    }
    
    return 0;
}

/* 删除定时器 */
int nv_loop_del_timer(nv_loop_t *loop, nv_event_ext_t *ev) {
    if (!loop || !ev) return -1;
    
    nv_loop_remove_event_from_list(&loop->timers.head, ev);
    ev->loop = NULL;
    
    return 0;
}

/* 添加空闲事件 */
int nv_loop_add_idle(nv_loop_t *loop, nv_event_ext_t *ev) {
    if (!loop || !ev) return -1;
    
    ev->type = NV_EVENT_TYPE_IDLE;
    ev->loop = loop;
    
    nv_loop_add_event_to_list(&loop->idle.head, ev);
    
    return 0;
}

/* 删除空闲事件 */
int nv_loop_del_idle(nv_loop_t *loop, nv_event_ext_t *ev) {
    if (!loop || !ev) return -1;
    
    nv_loop_remove_event_from_list(&loop->idle.head, ev);
    ev->loop = NULL;
    
    return 0;
}

/* 处理IO事件 */
static int nv_loop_process_io_events(nv_loop_t *loop) {
    for (int i = 0; i < loop->event_count; i++) {
        struct epoll_event *epoll_ev = &loop->events[i];
        nv_event_ext_t *ev = (nv_event_ext_t *)epoll_ev->data.ptr;
        
        if (!ev) continue; /* 唤醒事件 */
        
        /* 转换epoll事件到libnv事件 */
        int revents = 0;
        if (epoll_ev->events & EPOLLIN) revents |= NV_EVENT_READ;
        if (epoll_ev->events & EPOLLOUT) revents |= NV_EVENT_WRITE;
        if (epoll_ev->events & EPOLLERR) revents |= NV_EVENT_ERROR;
        if (epoll_ev->events & EPOLLHUP) revents |= NV_EVENT_HUP;
        
        /* 触发事件 */
        if (revents) {
            nv_event_trigger(ev, revents);
            loop->stats.events_processed++;
        }
    }
    
    return 0;
}

/* 处理定时器事件 */
static int nv_loop_process_timers(nv_loop_t *loop) {
    unsigned long now = nv_loop_get_monotonic_time();
    nv_event_ext_t *ev, *next;
    
    for (ev = loop->timers.head; ev; ev = next) {
        next = ev->next;
        
        if (ev->expire_time <= now) {
            /* 定时器过期，触发事件 */
            nv_event_trigger(ev, NV_EVENT_TIMER);
            loop->stats.timers_processed++;
            
            /* 从链表中移除 */
            nv_loop_remove_event_from_list(&loop->timers.head, ev);
            ev->loop = NULL;
        }
    }
    
    /* 更新下次超时时间 */
    loop->timers.next_timeout = 0;
    for (ev = loop->timers.head; ev; ev = ev->next) {
        if (!loop->timers.next_timeout || ev->expire_time < loop->timers.next_timeout) {
            loop->timers.next_timeout = ev->expire_time;
        }
    }
    
    return 0;
}

/* 处理信号事件 */
static int nv_loop_process_signals(nv_loop_t *loop) {
    /* 这里可以实现信号处理逻辑 */
    return 0;
}

/* 处理空闲事件 */
static int nv_loop_process_idle_events(nv_loop_t *loop) {
    nv_event_ext_t *ev, *next;
    
    for (ev = loop->idle.head; ev; ev = next) {
        next = ev->next;
        
        /* 触发空闲事件 */
        nv_event_trigger(ev, NV_EVENT_IDLE);
        loop->stats.idle_events_processed++;
    }
    
    return 0;
}

/* 添加事件到链表 */
static void nv_loop_add_event_to_list(nv_event_ext_t **head, nv_event_ext_t *ev) {
    if (!head || !ev) return;
    
    ev->next = *head;
    ev->prev = NULL;
    
    if (*head) {
        (*head)->prev = ev;
    }
    
    *head = ev;
}

/* 从链表移除事件 */
static void nv_loop_remove_event_from_list(nv_event_ext_t **head, nv_event_ext_t *ev) {
    if (!head || !ev) return;
    
    if (ev->prev) {
        ev->prev->next = ev->next;
    } else {
        *head = ev->next;
    }
    
    if (ev->next) {
        ev->next->prev = ev->prev;
    }
    
    ev->next = NULL;
    ev->prev = NULL;
}

/* 获取单调时间 */
static unsigned long nv_loop_get_monotonic_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

/* 获取事件循环统计信息 */
void nv_loop_get_stats(nv_loop_t *loop, nv_loop_stats_t *stats) {
    if (!loop || !stats) return;
    
    memcpy(stats, &loop->stats, sizeof(nv_loop_stats_t));
    
    /* 计算当前活动事件数 */
    stats->active_events = 0;
    nv_event_ext_t *ev;
    
    for (ev = loop->timers.head; ev; ev = ev->next) {
        if (ev->active) stats->active_events++;
    }
    
    for (ev = loop->idle.head; ev; ev = ev->next) {
        if (ev->active) stats->active_events++;
    }
}

/* 重置统计信息 */
void nv_loop_reset_stats(nv_loop_t *loop) {
    if (!loop) return;
    
    memset(&loop->stats, 0, sizeof(loop->stats));
}

/* 检查事件循环是否运行中 */
int nv_loop_is_running(nv_loop_t *loop) {
    return loop ? (loop->state == NV_LOOP_RUNNING) : 0;
}

/* 修改定时器 */
int nv_loop_modify_timer(nv_loop_t *loop, nv_event_ext_t *ev, unsigned long timeout_ms) {
    if (!loop || !ev) return -1;
    
    /* 先从定时器链表中移除 */
    nv_loop_remove_event_from_list(&loop->timers.head, ev);
    
    /* 重新设置超时时间 */
    ev->timeout_ms = timeout_ms;
    ev->expire_time = nv_loop_get_monotonic_time() + timeout_ms;
    
    /* 重新添加到定时器链表 */
    nv_loop_add_event_to_list(&loop->timers.head, ev);
    
    /* 更新下次超时时间 */
    if (!loop->timers.next_timeout || ev->expire_time < loop->timers.next_timeout) {
        loop->timers.next_timeout = ev->expire_time;
    }
    
    return 0;
}

/* 添加信号事件 */
int nv_loop_add_signal(nv_loop_t *loop, nv_event_ext_t *ev, int signo) {
    if (!loop || !ev) return -1;
    
    /* 设置信号事件属性 */
    ev->type = NV_EVENT_TYPE_SIGNAL;
    ev->loop = loop;
    
    /* 扩展信号数组 */
    int new_count = loop->signals.signal_count + 1;
    int *new_signals = realloc(loop->signals.signals, new_count * sizeof(int));
    nv_event_ext_t **new_signal_events = realloc(loop->signals.signal_events, new_count * sizeof(nv_event_ext_t*));
    
    if (!new_signals || !new_signal_events) {
        free(new_signals);
        free(new_signal_events);
        return -1;
    }
    
    loop->signals.signals = new_signals;
    loop->signals.signal_events = new_signal_events;
    
    /* 添加信号和事件 */
    loop->signals.signals[loop->signals.signal_count] = signo;
    loop->signals.signal_events[loop->signals.signal_count] = ev;
    loop->signals.signal_count = new_count;
    
    return 0;
}

/* 删除信号事件 */
int nv_loop_del_signal(nv_loop_t *loop, nv_event_ext_t *ev) {
    if (!loop || !ev) return -1;
    
    /* 查找并移除信号事件 */
    for (int i = 0; i < loop->signals.signal_count; i++) {
        if (loop->signals.signal_events[i] == ev) {
            /* 移动后面的元素 */
            for (int j = i; j < loop->signals.signal_count - 1; j++) {
                loop->signals.signals[j] = loop->signals.signals[j + 1];
                loop->signals.signal_events[j] = loop->signals.signal_events[j + 1];
            }
            loop->signals.signal_count--;
            ev->loop = NULL;
            return 0;
        }
    }
    
    return -1;
}

/* 忽略信号 */
int nv_loop_ignore_signal(nv_loop_t *loop, int signo) {
    if (!loop) return -1;
    
    /* 查找并移除指定的信号 */
    for (int i = 0; i < loop->signals.signal_count; i++) {
        if (loop->signals.signals[i] == signo) {
            /* 移动后面的元素 */
            for (int j = i; j < loop->signals.signal_count - 1; j++) {
                loop->signals.signals[j] = loop->signals.signals[j + 1];
                loop->signals.signal_events[j] = loop->signals.signal_events[j + 1];
            }
            loop->signals.signal_count--;
            return 0;
        }
    }
    
    return -1;
}

/* 暂停事件循环 */
int nv_loop_pause(nv_loop_t *loop) {
    if (!loop || loop->state != NV_LOOP_RUNNING) return -1;
    
    loop->state = NV_LOOP_STOPPING;
    return 0;
}

/* 恢复事件循环 */
int nv_loop_resume(nv_loop_t *loop) {
    if (!loop || loop->state != NV_LOOP_STOPPING) return -1;
    
    loop->state = NV_LOOP_RUNNING;
    return 0;
}

/* 设置事件循环配置 */
int nv_loop_set_config(nv_loop_t *loop, const nv_loop_config_t *config) {
    if (!loop || !config) return -1;
    
    /* 这里可以实现动态配置更新逻辑 */
    /* 当前实现只是简单返回成功 */
    return 0;
}

/* 获取事件循环配置 */
int nv_loop_get_config(nv_loop_t *loop, nv_loop_config_t *config) {
    if (!loop || !config) return -1;
    
    /* 填充配置结构 */
    config->max_events = loop->max_events;
    config->timeout_ms = 1000; /* 默认值 */
    config->max_fd = 65536;    /* 默认值 */
    config->use_epoll = 1;
    config->use_poll = 0;
    config->use_select = 0;
    config->enable_timers = (loop->timers.head != NULL);
    config->enable_signals = (loop->signals.signal_count > 0);
    config->enable_idle = (loop->idle.head != NULL);
    
    return 0;
}

/* 获取当前时间 */
unsigned long nv_loop_now(nv_loop_t *loop) {
    (void)loop; /* 避免未使用参数警告 */
    return nv_loop_get_monotonic_time();
}

/* 更新时间 */
int nv_loop_update_time(nv_loop_t *loop) {
    if (!loop) return -1;
    
    /* 这里可以实现时间更新逻辑 */
    /* 当前实现只是简单返回成功 */
    return 0;
}
