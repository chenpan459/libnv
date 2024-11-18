
#include "nv_timer_task.h"



// 初始化定时器管理器
nv_timer_manager_t* nv_timer_manager_init(size_t capacity) {
    nv_timer_manager_t* manager = (nv_timer_manager_t*)malloc(sizeof(nv_timer_manager_t));
    if (!manager) {
        perror("NV: Failed to allocate memory for timer manager");
        return NULL;
    }
    manager->timers = (nv_timer_t*)calloc(capacity, sizeof(nv_timer_t));
    if (!manager->timers) {
        perror("NV: Failed to allocate memory for timers");
        free(manager);
        return NULL;
    }
    manager->capacity = capacity;
    manager->size = 0;
    return manager;
}

// 创建定时器
int nv_timer_manager_create_timer(nv_timer_manager_t* manager, time_t seconds, long nanoseconds, nv_timer_callback_t callback, void* user_data) {
    if (manager->size >= manager->capacity) {
        fprintf(stderr, "NV: Timer manager is full\n");
        return -1;
    }

    nv_timer_t* timer = &manager->timers[manager->size];
    timer->fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer->fd == -1) {
        perror("NV: Failed to create timerfd");
        return -1;
    }

    struct itimerspec new_value;
    memset(&new_value, 0, sizeof(struct itimerspec));
    new_value.it_value.tv_sec = seconds;
    new_value.it_value.tv_nsec = nanoseconds;

    if (timerfd_settime(timer->fd, 0, &new_value, NULL) == -1) {
        perror("NV: Failed to set timer");
        close(timer->fd);
        return -1;
    }

    timer->callback = callback;
    timer->user_data = user_data;
    manager->size++;

    return timer->fd;
}

// 销毁定时器管理器
void nv_timer_manager_destroy(nv_timer_manager_t* manager) {
    for (size_t i = 0; i < manager->size; i++) {
        close(manager->timers[i].fd);
    }
    free(manager->timers);
    free(manager);
}

// 处理定时器超时
void nv_timer_manager_process_timers(nv_timer_manager_t* manager) {
    struct pollfd* fds = (struct pollfd*)malloc(manager->size * sizeof(struct pollfd));
    if (!fds) {
        perror("NV: Failed to allocate memory for pollfds");
        return;
    }

    for (size_t i = 0; i < manager->size; i++) {
        fds[i].fd = manager->timers[i].fd;
        fds[i].events = POLLIN;
    }

    while (1) {
        int ret = poll(fds, manager->size, -1);
        if (ret == -1) {
            perror("NV: poll");
            break;
        }

        for (size_t i = 0; i < manager->size; i++) {
            if (fds[i].revents & POLLIN) {
                uint64_t expirations;
                if (read(fds[i].fd, &expirations, sizeof(expirations)) == -1) {
                    perror("NV: read");
                    continue;
                }
                if (manager->timers[i].callback) {
                    manager->timers[i].callback(manager->timers[i].user_data);
                }
            }
        }
    }

    free(fds);
}

// 示例回调函数
void my_callback(void* user_data) {
    printf("NV: Timer expired! User data: %s\n", (char*)user_data);
}

int nv_timer_task_main() {
    // 初始化定时器管理器
    nv_timer_manager_t* manager = nv_timer_manager_init(10);
    if (!manager) {
        return EXIT_FAILURE;
    }

    // 创建多个定时器并分别设置超时时间
    nv_timer_manager_create_timer(manager, 1, 0, my_callback, "Timer 1");
    nv_timer_manager_create_timer(manager, 3, 0, my_callback, "Timer 2");
    nv_timer_manager_create_timer(manager, 5, 0, my_callback, "Timer 3");

    printf("NV: Timers set, waiting for expiration...\n");

    // 处理定时器超时
    nv_timer_manager_process_timers(manager);

    // 销毁定时器管理器
    nv_timer_manager_destroy(manager);

    return EXIT_SUCCESS;
}

