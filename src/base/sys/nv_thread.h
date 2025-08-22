#ifndef _NV_THREAD_H_INCLUDED_
#define _NV_THREAD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>

typedef pthread_t nv_pthread_t; // 线程句柄
typedef pthread_attr_t nv_pthread_attr_t; // 线程属性

#define nv_pthread_self() pthread_self()  // 获取当前线程 ID
#define nv_pthread_equal(a, b) pthread_equal(a, b)  // 判断两个线程是否相等

// 线程函数类型
typedef void* (*thread_func_t)(void*);

// 封装线程属性设置接口
int nv_init_thread_attr(nv_pthread_attr_t *attr, int detach_state, size_t stack_size);
// 带调度策略与优先级的扩展版本（policy 如 SCHED_OTHER/SCHED_FIFO/SCHED_RR）
int nv_init_thread_attr_ex(nv_pthread_attr_t *attr, int detach_state, size_t stack_size, int policy, int priority);

// 封装线程创建接口
int nv_create_thread(nv_pthread_t *thread, const nv_pthread_attr_t *attr, thread_func_t func, void *arg);

// 封装线程等待接口
int nv_join_thread(nv_pthread_t thread);
// 超时等待（毫秒）。返回0成功，ETIMEDOUT超时，其他为错误码
int nv_try_join_thread(nv_pthread_t thread, int timeout_ms);

// 封装线程分离接口
int nv_detach_thread(nv_pthread_t thread);

// 线程工具：设置线程名（最多15字符+终止符）
int nv_thread_set_name(nv_pthread_t thread, const char *name);
// 设置线程CPU亲和性（将线程绑定到单个CPU核）
int nv_thread_set_affinity(nv_pthread_t thread, int cpu_index);
// 设置线程优先级（policy 与 priority）
int nv_thread_set_priority(nv_pthread_t thread, int policy, int priority);
// 线程取消
int nv_thread_cancel(nv_pthread_t thread);
// 让出CPU
void nv_thread_yield(void);
// 睡眠毫秒
void nv_thread_sleep_ms(unsigned long ms);

#ifdef __cplusplus
}
#endif
#endif