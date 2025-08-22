#include "nv_thread_pool.h"

static int nv_tp_enqueue(threadpool_t *tp, task_t task) {
    if (tp->count == tp->queue_capacity) return NV_TP_EFULL;
    tp->queue[tp->tail] = task;
    tp->tail = (tp->tail + 1) % tp->queue_capacity;
    tp->count += 1;
    return NV_TP_OK;
}

static int nv_tp_dequeue(threadpool_t *tp, task_t *task) {
    if (tp->count == 0) return NV_TP_EINVAL;
    *task = tp->queue[tp->head];
    tp->head = (tp->head + 1) % tp->queue_capacity;
    tp->count -= 1;
    return NV_TP_OK;
}

threadpool_t *nv_threadpool_create(int thread_count, int queue_capacity) {
    if (thread_count <= 0 || queue_capacity <= 0) {
        return NULL;
    }

    threadpool_t *tp = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (!tp) return NULL;

    tp->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    tp->queue   = (task_t *)malloc(sizeof(task_t) * queue_capacity);
    if (!tp->threads || !tp->queue) {
        free(tp->threads); free(tp->queue); free(tp);
        return NULL;
    }

    tp->thread_count = thread_count;
    tp->queue_capacity = queue_capacity;
    tp->head = tp->tail = tp->count = 0;
    tp->shutdown = 0;
    tp->shutdown_mode = NV_TP_SHUTDOWN_GRACEFUL;

    pthread_mutex_init(&(tp->lock), NULL);
    pthread_cond_init(&(tp->not_empty), NULL);
    pthread_cond_init(&(tp->not_full), NULL);
    pthread_cond_init(&(tp->empty), NULL);

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&(tp->threads[i]), NULL, nv_threadpool_thread, (void *)tp);
    }

    return tp;
}

int nv_threadpool_add(threadpool_t *tp, void (*function)(void *), void *argument) {
    if (!tp || !function) return NV_TP_EINVAL;

    pthread_mutex_lock(&(tp->lock));

    while (!tp->shutdown && tp->count == tp->queue_capacity) {
        pthread_cond_wait(&(tp->not_full), &(tp->lock));
    }
    if (tp->shutdown) {
        pthread_mutex_unlock(&(tp->lock));
        return NV_TP_ESHUTDOWN;
    }

    task_t task; task.function = function; task.data = argument;
    int rc = nv_tp_enqueue(tp, task);
    if (rc == NV_TP_OK) {
        pthread_cond_signal(&(tp->not_empty));
    }

    pthread_mutex_unlock(&(tp->lock));
    return rc;
}

int nv_threadpool_add_timeout(threadpool_t *tp, void (*function)(void *), void *argument, const struct timespec *abstime) {
    if (!tp || !function) return NV_TP_EINVAL;

    pthread_mutex_lock(&(tp->lock));

    int wait_rc = 0;
    while (!tp->shutdown && tp->count == tp->queue_capacity) {
        if (!abstime) {
            pthread_cond_wait(&(tp->not_full), &(tp->lock));
        } else {
            wait_rc = pthread_cond_timedwait(&(tp->not_full), &(tp->lock), abstime);
            if (wait_rc == ETIMEDOUT) {
                pthread_mutex_unlock(&(tp->lock));
                return NV_TP_ETIMEDOUT;
            }
        }
    }
    if (tp->shutdown) {
        pthread_mutex_unlock(&(tp->lock));
        return NV_TP_ESHUTDOWN;
    }

    task_t task; task.function = function; task.data = argument;
    int rc = nv_tp_enqueue(tp, task);
    if (rc == NV_TP_OK) {
        pthread_cond_signal(&(tp->not_empty));
    }

    pthread_mutex_unlock(&(tp->lock));
    return rc;
}

int nv_threadpool_wait_idle(threadpool_t *tp) {
    if (!tp) return NV_TP_EINVAL;
    pthread_mutex_lock(&(tp->lock));
    while (tp->count != 0) {
        pthread_cond_wait(&(tp->empty), &(tp->lock));
    }
    pthread_mutex_unlock(&(tp->lock));
    return NV_TP_OK;
}

void *nv_threadpool_thread(void *threadpool) {
    threadpool_t *tp = (threadpool_t *)threadpool;
    task_t task;

    for (;;) {
        pthread_mutex_lock(&(tp->lock));

        while ((tp->count == 0) && (!tp->shutdown)) {
            pthread_cond_wait(&(tp->not_empty), &(tp->lock));
        }

        if (tp->shutdown) {
            if (tp->shutdown_mode == NV_TP_SHUTDOWN_IMMEDIATE) {
                pthread_mutex_unlock(&(tp->lock));
                break;
            }
            /* 优雅关闭：等待队列清空后退出 */
            if (tp->count == 0) {
                pthread_mutex_unlock(&(tp->lock));
                break;
            }
        }

        if (nv_tp_dequeue(tp, &task) == NV_TP_OK) {
            /* 出队后如果队列为空，通知等待者 */
            if (tp->count == 0) {
                pthread_cond_broadcast(&(tp->empty));
            }
            /* 出队释放一个放入名额 */
            pthread_cond_signal(&(tp->not_full));
            pthread_mutex_unlock(&(tp->lock));

            (*(task.function))(task.data);
        } else {
            pthread_mutex_unlock(&(tp->lock));
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int nv_threadpool_destroy(threadpool_t *tp, nv_tp_shutdown_mode_t mode) {
    if (!tp) return NV_TP_EINVAL;

    pthread_mutex_lock(&(tp->lock));
    tp->shutdown = 1;
    tp->shutdown_mode = mode;
    pthread_cond_broadcast(&(tp->not_empty));
    pthread_cond_broadcast(&(tp->not_full));
    pthread_mutex_unlock(&(tp->lock));

    for (int i = 0; i < tp->thread_count; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&(tp->lock));
    pthread_cond_destroy(&(tp->not_empty));
    pthread_cond_destroy(&(tp->not_full));
    pthread_cond_destroy(&(tp->empty));

    free(tp->threads);
    free(tp->queue);
    free(tp);

    return NV_TP_OK;
}

/* 示例函数保持但适配新API */
#include <stdio.h>
#include <unistd.h>
static void example_task(void *arg) {
    int num = *((int *)arg);
    printf("Executing task %d\n", num);
    usleep(1000 * 500);
}

int nv_thread_pool_main() {
    threadpool_t *tp = nv_threadpool_create(4, 16);
    int tasks[10] = {0,1,2,3,4,5,6,7,8,9};

    for (int i = 0; i < 10; i++) {
        nv_threadpool_add(tp, example_task, (void *)&tasks[i]);
    }

    nv_threadpool_wait_idle(tp);
    nv_threadpool_destroy(tp, NV_TP_SHUTDOWN_GRACEFUL);
    return 0;
}
