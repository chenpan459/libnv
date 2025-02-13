#include "nv_mmap.h"

// 打开内存映射文件
// 定义一个函数，用于打开并映射一个文件到内存
nv_mmap_file_t* nv_mmap_open(const char* filepath, size_t size) {
    // 分配内存用于存储文件映射信息
    nv_mmap_file_t* mmap_file = (nv_mmap_file_t*)malloc(sizeof(nv_mmap_file_t));
    // 检查内存分配是否成功
    if (!mmap_file) {
        // 如果内存分配失败，输出错误信息
        perror("malloc");
        // 返回NULL表示函数执行失败
        return NULL;
    }

    // 打开文件，以读写和创建模式，文件权限为0666（可读写）
    mmap_file->fd = open(filepath, O_RDWR | O_CREAT, 0666);
    // 检查文件是否成功打开
    if (mmap_file->fd == -1) {
        // 如果文件打开失败，输出错误信息
        perror("open");
        // 释放之前分配的内存
        free(mmap_file);
        // 返回NULL表示函数执行失败
        return NULL;
    }

    // 调整文件大小为指定的大小
    if (ftruncate(mmap_file->fd, size) == -1) {
        // 如果调整文件大小失败，输出错误信息
        perror("ftruncate");
        // 关闭文件描述符
        close(mmap_file->fd);
        // 释放之前分配的内存
        free(mmap_file);
        // 返回NULL表示函数执行失败
        return NULL;
    }

    // 将文件映射到内存，指定映射大小、保护权限和共享映射
    mmap_file->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_file->fd, 0);
    // 检查内存映射是否成功
    if (mmap_file->data == MAP_FAILED) {
        // 如果内存映射失败，输出错误信息
        perror("mmap");
        // 关闭文件描述符
        close(mmap_file->fd);
        // 释放之前分配的内存
        free(mmap_file);
        // 返回NULL表示函数执行失败
        return NULL;
    }

    mmap_file->size = size;
    return mmap_file;
}

// 读取数据
void nv_mmap_read(nv_mmap_file_t* mmap_file, size_t offset, void* buffer, size_t size) {
    if (offset + size > mmap_file->size) {
        fprintf(stderr, "Read out of bounds\n");
        return;
    }
    memcpy(buffer, (char*)mmap_file->data + offset, size);
}

// 写入数据
void nv_mmap_write(nv_mmap_file_t* mmap_file, size_t offset, const void* buffer, size_t size) {
    if (offset + size > mmap_file->size) {
        fprintf(stderr, "Write out of bounds\n");
        return;
    }
    memcpy((char*)mmap_file->data + offset, buffer, size);
}

// 关闭内存映射文件
void nv_mmap_close(nv_mmap_file_t* mmap_file) {
    if (mmap_file) {
        munmap(mmap_file->data, mmap_file->size);
        close(mmap_file->fd);
        free(mmap_file);
    }
}

int nv_mmap_main() {
    const char* filepath = "example_file.txt";
    size_t size = 1024;

    nv_mmap_file_t* mmap_file = nv_mmap_open(filepath, size);
    if (!mmap_file) {
        return 1;
    }

    const char* message = "Hello, mmap!";
    nv_mmap_write(mmap_file, 0, message, strlen(message) + 1);

    char buffer[50];
    nv_mmap_read(mmap_file, 0, buffer, sizeof(buffer));
    printf("Read from mmap: %s\n", buffer);

    nv_mmap_close(mmap_file);
    return 0;
}
