#ifndef _NV_THREAD_POOL_H_INCLUDED_
#define _NV_THREAD_POOL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>

/* 返回码 */
#define NV_TP_OK            0
#define NV_TP_EINVAL       -1
#define NV_TP_EFULL        -2
#define NV_TP_ESHUTDOWN    -3
#define NV_TP_ETIMEDOUT    -4

/* 关闭模式 */
typedef enum {
    NV_TP_SHUTDOWN_GRACEFUL = 0,   /* 等待队列任务执行完毕 */
    NV_TP_SHUTDOWN_IMMEDIATE = 1   /* 立即唤醒并退出，丢弃剩余任务 */
} nv_tp_shutdown_mode_t;

typedef struct {
    void (*function)(void *p);
    void *data;
} task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;      /* 有任务可取 */
    pthread_cond_t  not_full;       /* 有空间可放 */
    pthread_cond_t  empty;          /* 队列为空（用于等待所有任务完成） */

    pthread_t      *threads;
    task_t         *queue;

    int             thread_count;   /* 线程数 */
    int             queue_capacity; /* 队列容量 */
    int             head;           /* 取位置 */
    int             tail;           /* 放位置 */
    int             count;          /* 当前任务数 */

    int             shutdown;       /* 是否关闭 */
    nv_tp_shutdown_mode_t shutdown_mode;
} threadpool_t;

/* 创建/销毁 */
threadpool_t *nv_threadpool_create(int thread_count, int queue_capacity);
int nv_threadpool_destroy(threadpool_t *tp, nv_tp_shutdown_mode_t mode);

/* 任务提交 */
int nv_threadpool_add(threadpool_t *tp, void (*function)(void *), void *argument);
int nv_threadpool_add_timeout(threadpool_t *tp, void (*function)(void *), void *argument, const struct timespec *abstime);

/* 同步等待任务清空（所有已提交任务执行完） */
int nv_threadpool_wait_idle(threadpool_t *tp);

/* 工作线程入口（对外暴露用于测试/自定义） */
void *nv_threadpool_thread(void *threadpool);

/* 示例入口（可选） */
int nv_thread_pool_main();

#ifdef __cplusplus
}
#endif

#endif