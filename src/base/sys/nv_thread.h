#ifndef _NV_THREAD_H_INCLUDED_
#define _NV_THREAD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>



#include <pthread.h>

// 线程函数类型
typedef void* (*thread_func_t)(void*);

// 封装线程属性设置接口
int nv_init_thread_attr(pthread_attr_t *attr, int detach_state, size_t stack_size);

// 封装线程创建接口
int nv_create_thread(pthread_t *thread, const pthread_attr_t *attr, thread_func_t func, void *arg);

// 封装线程等待接口
int nv_join_thread(pthread_t thread);

// 封装线程分离接口
int nv_detach_thread(pthread_t thread);


#ifdef __cplusplus
}
#endif
#endif