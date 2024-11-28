#include "nv_thread.h"


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

// 封装线程创建接口
int nv_create_thread(nv_pthread_t *thread, const nv_pthread_attr_t *attr, thread_func_t func, void *arg) {
    return pthread_create(thread, attr, func, arg);
}

// 封装线程等待接口
int nv_join_thread(nv_pthread_t thread) {
    return pthread_join(thread, NULL);
}

// 封装线程分离接口
int nv_detach_thread(nv_pthread_t thread) {
    return pthread_detach(thread);
}















// 线程函数
void* thread_func(void* arg) {
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

    // 等待线程结束
    if (nv_join_thread(thread) != 0) {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }

    printf("Thread joined successfully\n");
    return 0;
}