#ifndef _NV_THREAD_H_INCLUDED_
#define _NV_THREAD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <pthread.h>

typedef pthread_t nv_pthread_t; // 线程句柄
typedef pthread_attr_t nv_pthread_attr_t; // 线程属性

#define nv_pthread_self() pthread_self()  // 获取当前线程 ID
#define nv_pthread_equal(a, b) pthread_equal(a, b)  // 判断两个线程是否相等

// 线程函数类型
typedef void* (*thread_func_t)(void*);

// 封装线程属性设置接口
int nv_init_thread_attr(nv_pthread_attr_t *attr, int detach_state, size_t stack_size);

// 封装线程创建接口
int nv_create_thread(nv_pthread_t *thread, const nv_pthread_attr_t *attr, thread_func_t func, void *arg);

// 封装线程等待接口
int nv_join_thread(nv_pthread_t thread);

// 封装线程分离接口
int nv_detach_thread(nv_pthread_t thread);


#ifdef __cplusplus
}
#endif
#endif