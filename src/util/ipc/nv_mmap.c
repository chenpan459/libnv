#include "nv_mmap.h"

// 打开文件并建立内存映射
nv_mmap_t* nv_mmap_open(const char* filepath) {
    nv_mmap_t* mmap = (nv_mmap_t*)malloc(sizeof(nv_mmap_t));
    if (!mmap) {
        perror("NV: Failed to allocate memory for mmap object");
        return NULL;
    }

    // 打开文件
    mmap->fd = open(filepath, O_RDWR);
    if (mmap->fd == -1) {
        perror("NV: Failed to open file");
        free(mmap);
        return NULL;
    }

    // 获取文件大小
    struct stat sb;
    if (fstat(mmap->fd, &sb) == -1) {
        perror("NV: Failed to get file size");
        close(mmap->fd);
        free(mmap);
        return NULL;
    }
    mmap->length = sb.st_size;

    // 建立内存映射
    mmap->addr = mmap(NULL, mmap->length, PROT_READ | PROT_WRITE, MAP_SHARED, mmap->fd, 0);
    if (mmap->addr == MAP_FAILED) {
        perror("NV: Failed to mmap file");
        close(mmap->fd);
        free(mmap);
        return NULL;
    }

    return mmap;
}

// 取消内存映射并关闭文件
void nv_mmap_close(nv_mmap_t* mmap) {
    if (mmap) {
        if (munmap(mmap->addr, mmap->length) == -1) {
            perror("NV: Failed to unmap file");
        }
        close(mmap->fd);
        free(mmap);
    }
}

// 获取映射区域的地址和长度
void* nv_mmap_get_addr(nv_mmap_t* mmap) {
    return mmap ? mmap->addr : NULL;
}

size_t nv_mmap_get_length(nv_mmap_t* mmap) {
    return mmap ? mmap->length : 0;
}





int nv_mmap_main() {
    const char* filepath = "test.txt"; // 修改为你的文件路径

    // 打开文件并建立内存映射
    nv_mmap_t* mmap = nv_mmap_open(filepath);
    if (!mmap) {
        return EXIT_FAILURE;
    }

    // 获取映射区域的地址和长度
    void* addr = nv_mmap_get_addr(mmap);
    size_t length = nv_mmap_get_length(mmap);

    // 读取并打印文件内容
    printf("File content:\n%s\n", (char*)addr);

    // 修改文件内容
    strncpy((char*)addr, "Hello, mmap!", length);

    // 取消内存映射并关闭文件
    nv_mmap_close(mmap);

    return EXIT_SUCCESS;
}
