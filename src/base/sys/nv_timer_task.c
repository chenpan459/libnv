
#include "nv_timer_task.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static int nv_timerfd_arm(int fd,
                          time_t seconds, long nanoseconds,
                          time_t interval_seconds, long interval_nanoseconds) {
    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = nanoseconds;
    its.it_interval.tv_sec = interval_seconds;
    its.it_interval.tv_nsec = interval_nanoseconds;
    return timerfd_settime(fd, 0, &its, NULL);
}

nv_timer_manager_t* nv_timer_manager_init(size_t capacity) {
    nv_timer_manager_t* manager = (nv_timer_manager_t*)malloc(sizeof(nv_timer_manager_t));
    if (!manager) {
        perror("NV: allocate timer manager");
        return NULL;
    }
    memset(manager, 0, sizeof(*manager));

    manager->epoll_fd = epoll_create1(0);
    if (manager->epoll_fd < 0) {
        perror("NV: epoll_create1");
        free(manager);
        return NULL;
    }

    manager->timers = (nv_timer_t*)calloc(capacity, sizeof(nv_timer_t));
    if (!manager->timers) {
        perror("NV: allocate timers");
        close(manager->epoll_fd);
        free(manager);
        return NULL;
    }
    manager->capacity = capacity;
    manager->size = 0;
    pthread_mutex_init(&manager->lock, NULL);
    manager->running = 0;

    return manager;
}

void nv_timer_manager_destroy(nv_timer_manager_t* manager) {
    if (!manager) return;
    pthread_mutex_lock(&manager->lock);
    for (size_t i = 0; i < manager->capacity; i++) {
        if (manager->timers[i].fd > 0) {
            epoll_ctl(manager->epoll_fd, EPOLL_CTL_DEL, manager->timers[i].fd, NULL);
            close(manager->timers[i].fd);
            manager->timers[i].fd = -1;
        }
    }
    pthread_mutex_unlock(&manager->lock);

    close(manager->epoll_fd);
    pthread_mutex_destroy(&manager->lock);
    free(manager->timers);
    free(manager);
}

static int nv_timer_manager_index_free(nv_timer_manager_t* manager) {
    for (size_t i = 0; i < manager->capacity; i++) {
        if (manager->timers[i].fd <= 0) return (int)i;
    }
    return -1;
}

int nv_timer_manager_create_timer(nv_timer_manager_t* manager,
                                  time_t seconds, long nanoseconds,
                                  time_t interval_seconds, long interval_nanoseconds,
                                  nv_timer_callback_t callback, void* user_data) {
    if (!manager || !callback) return -1;

    pthread_mutex_lock(&manager->lock);
    int idx = nv_timer_manager_index_free(manager);
    if (idx < 0) {
        pthread_mutex_unlock(&manager->lock);
        fprintf(stderr, "NV: timer capacity full\n");
        return -1;
    }

    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0) {
        pthread_mutex_unlock(&manager->lock);
        perror("NV: timerfd_create");
        return -1;
    }

    if (nv_timerfd_arm(fd, seconds, nanoseconds, interval_seconds, interval_nanoseconds) < 0) {
        pthread_mutex_unlock(&manager->lock);
        perror("NV: timerfd_settime");
        close(fd);
        return -1;
    }

    manager->timers[idx].fd = fd;
    manager->timers[idx].callback = callback;
    manager->timers[idx].user_data = user_data;
    manager->timers[idx].kind = (interval_seconds || interval_nanoseconds) ? NV_TIMER_PERIODIC : NV_TIMER_ONESHOT;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.u32 = (uint32_t)idx; /* 将索引放入u32，容量需<= UINT32_MAX */
    if (epoll_ctl(manager->epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("NV: epoll_ctl ADD");
        close(fd);
        manager->timers[idx].fd = -1;
        pthread_mutex_unlock(&manager->lock);
        return -1;
    }

    if ((size_t)idx >= manager->size) manager->size = (size_t)idx + 1;
    pthread_mutex_unlock(&manager->lock);
    return fd;
}

int nv_timer_manager_cancel(nv_timer_manager_t* manager, int timer_fd) {
    if (!manager || timer_fd <= 0) return -1;
    pthread_mutex_lock(&manager->lock);
    for (size_t i = 0; i < manager->capacity; i++) {
        if (manager->timers[i].fd == timer_fd) {
            epoll_ctl(manager->epoll_fd, EPOLL_CTL_DEL, timer_fd, NULL);
            close(timer_fd);
            manager->timers[i].fd = -1;
            manager->timers[i].callback = NULL;
            manager->timers[i].user_data = NULL;
            manager->timers[i].kind = NV_TIMER_ONESHOT;
            pthread_mutex_unlock(&manager->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&manager->lock);
    return -1;
}

int nv_timer_manager_modify(nv_timer_manager_t* manager,
                            int timer_fd,
                            time_t seconds, long nanoseconds,
                            time_t interval_seconds, long interval_nanoseconds) {
    if (!manager || timer_fd <= 0) return -1;
    pthread_mutex_lock(&manager->lock);
    for (size_t i = 0; i < manager->capacity; i++) {
        if (manager->timers[i].fd == timer_fd) {
            int rc = nv_timerfd_arm(timer_fd, seconds, nanoseconds, interval_seconds, interval_nanoseconds);
            if (rc == 0) {
                manager->timers[i].kind = (interval_seconds || interval_nanoseconds) ? NV_TIMER_PERIODIC : NV_TIMER_ONESHOT;
            }
            pthread_mutex_unlock(&manager->lock);
            return rc;
        }
    }
    pthread_mutex_unlock(&manager->lock);
    return -1;
}

int nv_timer_manager_process_once(nv_timer_manager_t* manager, int timeout_ms) {
    if (!manager) return -1;

    struct epoll_event evs[64];
    int wait_ms = timeout_ms;
    if (timeout_ms < 0) wait_ms = -1;

    int n = epoll_wait(manager->epoll_fd, evs, (int)(sizeof(evs)/sizeof(evs[0])), wait_ms);
    if (n < 0) {
        if (errno == EINTR) return 0; /* 被信号中断，认为无事件 */
        perror("NV: epoll_wait");
        return -1;
    }

    for (int i = 0; i < n; i++) {
        uint32_t idx = evs[i].data.u32;
        pthread_mutex_lock(&manager->lock);
        if (idx >= manager->capacity || manager->timers[idx].fd <= 0) {
            pthread_mutex_unlock(&manager->lock);
            continue;
        }
        int fd = manager->timers[idx].fd;
        nv_timer_callback_t cb = manager->timers[idx].callback;
        void* ud = manager->timers[idx].user_data;
        nv_timer_kind_t kind = manager->timers[idx].kind;
        pthread_mutex_unlock(&manager->lock);

        uint64_t expirations;
        if (read(fd, &expirations, sizeof(expirations)) < 0) {
            if (errno != EAGAIN) perror("NV: read timerfd");
            continue;
        }
        if (cb) cb(ud);

        if (kind == NV_TIMER_ONESHOT) {
            /* 一次性定时器触发后自动取消 */
            nv_timer_manager_cancel(manager, fd);
        }
    }

    return n;
}

int nv_timer_manager_run(nv_timer_manager_t* manager) {
    if (!manager) return -1;
    manager->running = 1;
    while (manager->running) {
        int rc = nv_timer_manager_process_once(manager, -1);
        if (rc < 0) return rc;
    }
    return 0;
}

void nv_timer_manager_stop(nv_timer_manager_t* manager) {
    if (!manager) return;
    manager->running = 0;
}

/* 示例回调与 main 可按需保留 */
static void my_callback(void* user_data) {
    printf("NV: Timer expired! User data: %s\n", (char*)user_data);
}

int nv_timer_task_main() {
    nv_timer_manager_t* manager = nv_timer_manager_init(128);
    if (!manager) return EXIT_FAILURE;

    int t1 = nv_timer_manager_create_timer(manager, 1, 0, 0, 0, my_callback, "Timer once 1s");
    (void)t1;
    int t2 = nv_timer_manager_create_timer(manager, 1, 0, 1, 0, my_callback, "Timer periodic 1s");
    (void)t2;

    printf("NV: Timers set, running...\n");

    /* 单步处理5秒演示 */
    for (int i = 0; i < 5; i++) {
        nv_timer_manager_process_once(manager, 1000);
    }

    nv_timer_manager_destroy(manager);
    return EXIT_SUCCESS;
}

