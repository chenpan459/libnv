#ifndef _NV_THREAD_H_INCLUDED_
#define _NV_THREAD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"

// 定义函数指针类型
typedef void* (*thread_func_t)(void*);

int nv_init_thread_attr(pthread_attr_t *attr, int detach_state, size_t stack_size) ;
int nv_create_thread(pthread_t *thread, const pthread_attr_t *attr, thread_func_t func, void *arg) ;
int nv_join_thread(pthread_t thread) ;

#ifdef __cplusplus
}
#endif
#endif