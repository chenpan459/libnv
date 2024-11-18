#include "nv_semaphore.h"



// 创建/打开信号量
nv_semaphore_t* nv_semaphore_open(const char* name, unsigned int value) {
    nv_semaphore_t* sem_obj = (nv_semaphore_t*)malloc(sizeof(nv_semaphore_t));
    if (!sem_obj) {
        perror("NV: Failed to allocate memory for semaphore");
        return NULL;
    }

    // 创建信号量
    sem_obj->sem = sem_open(name, O_CREAT, 0644, value);
    if (sem_obj->sem == SEM_FAILED) {
        perror("NV: Failed to open semaphore");
        free(sem_obj);
        return NULL;
    }

    return sem_obj;
}

// 关闭信号量
int nv_semaphore_close(nv_semaphore_t* sem_obj) {
    if (!sem_obj) {
        perror("NV: Invalid semaphore object");
        return -1;
    }

    if (sem_close(sem_obj->sem) == -1) {
        perror("NV: Failed to close semaphore");
        return -1;
    }

    free(sem_obj);
    return 0;
}

// 删除信号量
int nv_semaphore_unlink(const char* name) {
    if (sem_unlink(name) == -1) {
        perror("NV: Failed to unlink semaphore");
        return -1;
    }

    return 0;
}

// 等待信号量（P操作）
int nv_semaphore_wait(nv_semaphore_t* sem_obj) {
    if (!sem_obj) {
        perror("NV: Invalid semaphore object");
        return -1;
    }

    if (sem_wait(sem_obj->sem) == -1) {
        perror("NV: Failed to wait semaphore");
        return -1;
    }

    return 0;
}

// 释放信号量（V操作）
int nv_semaphore_post(nv_semaphore_t* sem_obj) {
    if (!sem_obj) {
        perror("NV: Invalid semaphore object");
        return -1;
    }

    if (sem_post(sem_obj->sem) == -1) {
        perror("NV: Failed to post semaphore");
        return -1;
    }

    return 0;
}









// 假设已经定义了nv_semaphore_t结构和上述的函数
void* thread_function_semaphore(void* arg) {
    nv_semaphore_t* sem = (nv_semaphore_t*)arg;

    // 等待信号量
    if (nv_semaphore_wait(sem) == -1) {
        perror("Thread failed to wait semaphore");
        return NULL;
    }

    printf("Thread has entered the critical section.\n");
    sleep(2); // 模拟工作在临界区
    printf("Thread is leaving the critical section.\n");

    // 释放信号量
    if (nv_semaphore_post(sem) == -1) {
        perror("Thread failed to post semaphore");
        return NULL;
    }

    return NULL;
}

int nv_semaphore_main() {
    // 创建信号量，初始值为1
    nv_semaphore_t* sem = nv_semaphore_open(SEM_NAME, 1);
    if (!sem) {
        return EXIT_FAILURE;
    }

    pthread_t thread;

    // 创建一个线程
    if (pthread_create(&thread, NULL, thread_function_semaphore, (void*)sem) != 0) {
        perror("Failed to create thread");
        nv_semaphore_close(sem);
        nv_semaphore_unlink(SEM_NAME);
        return EXIT_FAILURE;
    }

    // 主线程尝试进入临界区
    if (nv_semaphore_wait(sem) == -1) {
        perror("Main thread failed to wait semaphore");
        nv_semaphore_close(sem);
        nv_semaphore_unlink(SEM_NAME);
        return EXIT_FAILURE;
    }

    printf("Main thread has entered the critical section.\n");
    // 主线程工作在临界区
    sleep(1);
    printf("Main thread is leaving the critical section.\n");

    // 释放信号量
    if (nv_semaphore_post(sem) == -1) {
        perror("Main thread failed to post semaphore");
        nv_semaphore_close(sem);
        nv_semaphore_unlink(SEM_NAME);
        return EXIT_FAILURE;
    }

    // 等待子线程结束
    pthread_join(thread, NULL);

    // 关闭并删除信号量
    nv_semaphore_close(sem);
    nv_semaphore_unlink(SEM_NAME);

    return EXIT_SUCCESS;
}


