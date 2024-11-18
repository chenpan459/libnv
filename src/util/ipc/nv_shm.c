#include "nv_shm.h"




// 创建/打开共享内存对象
nv_shm_t* nv_shm_open() {
    nv_shm_t *shm_obj = (nv_shm_t*)malloc(sizeof(nv_shm_t));
    if (!shm_obj) {
        perror("NV: Failed to allocate memory for shared memory object");
        return NULL;
    }

    // 创建共享内存对象
    shm_obj->shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_obj->shm_fd == -1) {
        perror("NV: Failed to create shared memory object");
        free(shm_obj);
        return NULL;
    }

    // 设置共享内存对象的大小
    if (ftruncate(shm_obj->shm_fd, SHM_SIZE) == -1) {
        perror("NV: Failed to set shared memory size");
        close(shm_obj->shm_fd);
        free(shm_obj);
        return NULL;
    }

    return shm_obj;
}

// 关联共享内存到进程的地址空间
void* nv_shm_map(nv_shm_t *shm_obj) {
    if (!shm_obj) {
        perror("NV: Invalid shared memory object");
        return NULL;
    }

    // 将共享内存对象映射到进程的地址空间
    void *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_obj->shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("NV: Failed to map shared memory");
        return NULL;
    }

    shm_obj->shm_ptr = shm_ptr;
    return shm_ptr;
}

// 取消关联共享内存
int nv_shm_unmap(nv_shm_t *shm_obj) {
    if (!shm_obj || !shm_obj->shm_ptr) {
        perror("NV: Invalid shared memory object or pointer");
        return -1;
    }

    if (munmap(shm_obj->shm_ptr, SHM_SIZE) == -1) {
        perror("NV: Failed to unmap shared memory");
        return -1;
    }

    shm_obj->shm_ptr = NULL;
    return 0;
}

// 关闭共享内存对象
int nv_shm_close(nv_shm_t *shm_obj) {
    if (!shm_obj) {
        perror("NV: Invalid shared memory object");
        return -1;
    }

    if (close(shm_obj->shm_fd) == -1) {
        perror("NV: Failed to close shared memory object");
        return -1;
    }

    return 0;
}

// 删除共享内存对象
int nv_shm_unlink() {
    if (shm_unlink(SHM_NAME) == -1) {
        perror("NV: Failed to unlink shared memory object");
        return -1;
    }

    return 0;
}





int nv_shm_main() {
    // 创建并打开共享内存对象
    nv_shm_t *shm_obj = nv_shm_open();
    if (!shm_obj) {
        return EXIT_FAILURE;
    }

    // 将共享内存对象映射到进程的地址空间
    void *shm_ptr = nv_shm_map(shm_obj);
    if (!shm_ptr) {
        nv_shm_close(shm_obj);
        return EXIT_FAILURE;
    }

    // 写入数据到共享内存
    const char *message = "Hello, Shared Memory!";
    strcpy((char *)shm_ptr, message);

    // 读取数据并打印
    printf("Message from shared memory: %s\n", (char *)shm_ptr);

    // 取消关联共享内存
    if (nv_shm_unmap(shm_obj) == -1) {
        nv_shm_close(shm_obj);
        return EXIT_FAILURE;
    }

    // 关闭共享内存对象
    if (nv_shm_close(shm_obj) == -1) {
        return EXIT_FAILURE;
    }

    // 删除共享内存对象
    if (nv_shm_unlink() == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
