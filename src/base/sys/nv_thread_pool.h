#ifndef _NV_THREAD_POOL_H_INCLUDED_
#define _NV_THREAD_POOL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>


#define THREAD_POOL_SIZE 4
#define QUEUE_SIZE 10

typedef struct {
    void (*function)(void *p);
    void *data;
} task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t threads[THREAD_POOL_SIZE];
    task_t queue[QUEUE_SIZE];
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
} threadpool_t;

threadpool_t *nv_threadpool_create();
int nv_threadpool_add(threadpool_t *tp, void (*function)(void *), void *argument);
void *nv_threadpool_thread(void *threadpool);
int nv_threadpool_destroy(threadpool_t *tp);




int nv_thread_pool_main();

#ifdef __cplusplus
}
#endif

#endif