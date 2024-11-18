#include "nv_thread_pool.h"




threadpool_t *nv_threadpool_create() {
    threadpool_t *tp;
    int i;

    if ((tp = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        return NULL;
    }

    tp->queue_size = QUEUE_SIZE;
    tp->head = tp->tail = tp->count = 0;
    tp->shutdown = 0;

    pthread_mutex_init(&(tp->lock), NULL);
    pthread_cond_init(&(tp->notify), NULL);

    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&(tp->threads[i]), NULL, nv_threadpool_thread, (void *)tp);
    }

    return tp;
}

int nv_threadpool_add(threadpool_t *tp, void (*function)(void *), void *argument) {
    int err = 0;

    pthread_mutex_lock(&(tp->lock));

    if (tp->count == tp->queue_size) {
        err = -1;
    } else {
        tp->queue[tp->tail].function = function;
        tp->queue[tp->tail].data = argument;
        tp->tail = (tp->tail + 1) % tp->queue_size;
        tp->count += 1;
        pthread_cond_signal(&(tp->notify));
    }

    pthread_mutex_unlock(&(tp->lock));
    return err;
}

void *nv_threadpool_thread(void *threadpool) {
    threadpool_t *tp = (threadpool_t *)threadpool;
    task_t task;

    while (1) {
        pthread_mutex_lock(&(tp->lock));

        while ((tp->count == 0) && (!tp->shutdown)) {
            pthread_cond_wait(&(tp->notify), &(tp->lock));
        }

        if (tp->shutdown) {
            break;
        }

        task.function = tp->queue[tp->head].function;
        task.data = tp->queue[tp->head].data;
        tp->head = (tp->head + 1) % tp->queue_size;
        tp->count -= 1;

        pthread_mutex_unlock(&(tp->lock));

        (*(task.function))(task.data);
    }

    pthread_mutex_unlock(&(tp->lock));
    pthread_exit(NULL);
    return NULL;
}

int nv_threadpool_destroy(threadpool_t *tp) {
    int i;

    pthread_mutex_lock(&(tp->lock));
    tp->shutdown = 1;
    pthread_cond_broadcast(&(tp->notify));
    pthread_mutex_unlock(&(tp->lock));

    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&(tp->lock));
    pthread_cond_destroy(&(tp->notify));
    free(tp);

    return 0;
}



void example_task(void *arg) {
    int num = *((int *)arg);
    printf("Executing task %d\n", num);
    sleep(1);
}

int nv_thread_pool_main() {
    threadpool_t *tp = nv_threadpool_create();
    int tasks[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    for (int i = 0; i < 10; i++) {
        nv_threadpool_add(tp, example_task, (void *)&tasks[i]);
    }

    sleep(5); // 等待所有任务完成
    nv_threadpool_destroy(tp);

    return 0;
}
