#include "nv_mmap.h"

// 打开内存映射文件
nv_mmap_file_t* nv_mmap_open(const char* filepath, size_t size) {
    nv_mmap_file_t* mmap_file = (nv_mmap_file_t*)malloc(sizeof(nv_mmap_file_t));
    if (!mmap_file) {
        perror("malloc");
        return NULL;
    }

    mmap_file->fd = open(filepath, O_RDWR | O_CREAT, 0666);
    if (mmap_file->fd == -1) {
        perror("open");
        free(mmap_file);
        return NULL;
    }

    if (ftruncate(mmap_file->fd, size) == -1) {
        perror("ftruncate");
        close(mmap_file->fd);
        free(mmap_file);
        return NULL;
    }

    mmap_file->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_file->fd, 0);
    if (mmap_file->data == MAP_FAILED) {
        perror("mmap");
        close(mmap_file->fd);
        free(mmap_file);
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
