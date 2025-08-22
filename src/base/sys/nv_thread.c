#include "nv_thread.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

// 封装线程属性设置接口
int nv_init_thread_attr(nv_pthread_attr_t *attr, int detach_state, size_t stack_size) {
    if (pthread_attr_init(attr) != 0) {
        return -1;
    }

    if (pthread_attr_setdetachstate(attr, detach_state) != 0) {
        pthread_attr_destroy(attr);
        return -1;
    }

    if (stack_size > 0 && pthread_attr_setstacksize(attr, stack_size) != 0) {
        pthread_attr_destroy(attr);
        return -1;
    }

    return 0;
}

// 扩展：包含调度策略与优先级
int nv_init_thread_attr_ex(nv_pthread_attr_t *attr, int detach_state, size_t stack_size, int policy, int priority) {
    if (nv_init_thread_attr(attr, detach_state, stack_size) != 0) {
        return -1;
    }

    if (policy == SCHED_OTHER || policy == SCHED_FIFO || policy == SCHED_RR) {
        struct sched_param sp;
        memset(&sp, 0, sizeof(sp));
        sp.sched_priority = priority;
        if (pthread_attr_setschedpolicy(attr, policy) != 0) {
            pthread_attr_destroy(attr);
            return -1;
        }
        if (pthread_attr_setschedparam(attr, &sp) != 0) {
            pthread_attr_destroy(attr);
            return -1;
        }
        // 明确指定使用显式调度参数
        if (pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) != 0) {
            pthread_attr_destroy(attr);
            return -1;
        }
    }
    return 0;
}

// 封装线程创建接口
int nv_create_thread(nv_pthread_t *thread, const nv_pthread_attr_t *attr, thread_func_t func, void *arg) {
    return pthread_create(thread, attr, func, arg);
}

// 封装线程等待接口
int nv_join_thread(nv_pthread_t thread) {
    return pthread_join(thread, NULL);
}

// 超时等待
int nv_try_join_thread(nv_pthread_t thread, int timeout_ms) {
#if defined(__linux__)
    if (timeout_ms < 0) {
        return EINVAL;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }
    return pthread_timedjoin_np(thread, NULL, &ts);
#else
    (void)thread; (void)timeout_ms;
    return ENOTSUP;
#endif
}

// 封装线程分离接口
int nv_detach_thread(nv_pthread_t thread) {
    return pthread_detach(thread);
}

// 线程工具：设置线程名
int nv_thread_set_name(nv_pthread_t thread, const char *name) {
#if defined(__linux__)
    // pthread_setname_np: 名称最多15字符
    return pthread_setname_np(thread, name);
#else
    (void)thread; (void)name;
    return ENOTSUP;
#endif
}

// 设置线程CPU亲和性
int nv_thread_set_affinity(nv_pthread_t thread, int cpu_index) {
#if defined(__linux__)
    if (cpu_index < 0) return EINVAL;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET((unsigned)cpu_index, &cpuset);
    return pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#else
    (void)thread; (void)cpu_index;
    return ENOTSUP;
#endif
}

// 设置线程优先级
int nv_thread_set_priority(nv_pthread_t thread, int policy, int priority) {
#if defined(__linux__)
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = priority;
    if (pthread_setschedparam(thread, policy, &sp) != 0) {
        return -1;
    }
    return 0;
#else
    (void)thread; (void)policy; (void)priority;
    return ENOTSUP;
#endif
}

// 线程取消
int nv_thread_cancel(nv_pthread_t thread) {
    return pthread_cancel(thread);
}

// 让出CPU
void nv_thread_yield(void) {
    sched_yield();
}

// 睡眠毫秒
void nv_thread_sleep_ms(unsigned long ms) {
    usleep(ms * 1000);
}

// 示例保留
#include <stdio.h>
static void* thread_func(void* arg) {
    (void)arg;
    printf("Hello from thread!\n");
    return NULL;
}

int nv_thread_main() {
    pthread_t thread;
    pthread_attr_t attr;

    // 初始化线程属性
    if (nv_init_thread_attr(&attr, PTHREAD_CREATE_JOINABLE, 0) != 0) {
        fprintf(stderr, "Failed to initialize thread attributes\n");
        return 1;
    }

    // 创建线程
    if (nv_create_thread(&thread, &attr, thread_func, NULL) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        pthread_attr_destroy(&attr);
        return 1;
    }

    // 销毁线程属性
    pthread_attr_destroy(&attr);

    // 等待线程结束（演示超时接口）
    int rc = nv_try_join_thread(thread, 2000);
    if (rc == ETIMEDOUT) {
        // 超时则执行阻塞join
        nv_join_thread(thread);
    } else if (rc != 0) {
        fprintf(stderr, "Failed to timed-join thread, rc=%d\n", rc);
        return 1;
    }

    printf("Thread joined successfully\n");
    return 0;
}