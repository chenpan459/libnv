#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "nv_lock.h"

// 定义互斥锁类型
typedef pthread_mutex_t nv_mutex_t;

// 初始化互斥锁
int nv_mutex_init(nv_mutex_t *mutex) {
    return pthread_mutex_init(mutex, NULL);
}

// 销毁互斥锁
int nv_mutex_destroy(nv_mutex_t *mutex) {
    return pthread_mutex_destroy(mutex);
}

// 锁定互斥锁
int nv_mutex_lock(nv_mutex_t *mutex) {
    return pthread_mutex_lock(mutex);
}

// 尝试锁定互斥锁（非阻塞）
int nv_mutex_trylock(nv_mutex_t *mutex) {
    return pthread_mutex_trylock(mutex);
}

// 解锁互斥锁
int nv_mutex_unlock(nv_mutex_t *mutex) {
    return pthread_mutex_unlock(mutex);
}

