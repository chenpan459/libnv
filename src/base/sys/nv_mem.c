#include "nv_mem.h"

// 封装 malloc 函数
// 参数: size - 要分配的内存大小（以字节为单位）
// 返回值: 分配的内存指针，如果分配失败则退出程序
void* nv_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// 封装 calloc 函数
// 参数: nmemb - 要分配的元素数量
//       size - 每个元素的大小（以字节为单位）
// 返回值: 分配并初始化为零的内存指针，如果分配失败则退出程序
void* nv_calloc(size_t nmemb, size_t size) {
    void* ptr = calloc(nmemb, size);
    if (!ptr) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// 封装 realloc 函数
// 参数: ptr - 指向要重新分配的内存的指针
//       size - 新的内存大小（以字节为单位）
// 返回值: 重新分配的内存指针，如果分配失败则退出程序
void* nv_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

// 封装 free 函数
// 参数: ptr - 指向要释放的内存的指针
void nv_free(void* ptr) {
    free(ptr);
}