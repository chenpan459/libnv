#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nv_thread.h"

// 定义函数指针类型
typedef void* (*thread_func_t)(void*);

// 封装线程属性设置接口
int nv_init_thread_attr(pthread_attr_t *attr, int detach_state, size_t stack_size) {
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

// 封装线程创建接口
int nv_create_thread(pthread_t *thread, const pthread_attr_t *attr, thread_func_t func, void *arg) {
    return pthread_create(thread, attr, func, arg);
}

// 封装线程等待接口
int nv_join_thread(pthread_t thread) {
    return pthread_join(thread, NULL);
}

// 示例线程函数
void* thread_function(void* arg) {
    int thread_num = *((int*)arg);
    printf("线程 %d 正在执行...\n", thread_num);
    sleep(1); // 模拟线程工作
    printf("线程 %d 执行完毕。\n", thread_num);
    return NULL;
}

