#ifndef _NV_MMAP_H_INCLUDED_
#define _NV_MMAP_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_util_include.h"

#include <sys/mman.h>  // 包含 mmap 函数的声明
#include <fcntl.h>     // 包含 O_* 常量
#include <unistd.h>    // 包含 close 函数的声明
#include <sys/stat.h>  // 包含 fstat 函数的声明
#include <string.h>    // 包含 strlen 和 memcpy 函数的声明
#include <stdio.h>     // 包含 perror 和 printf 函数的声明
#include <stdlib.h>    // 包含 exit 函数的声明

// 你的其他代码...


typedef struct {
    int fd;
    void* addr;
    size_t length;
} nv_mmap_t;


nv_mmap_t* nv_mmap_open(const char* filepath) ;
// 取消内存映射并关闭文件
void nv_mmap_close(nv_mmap_t* mmap) ;
// 获取映射区域的地址和长度
void* nv_mmap_get_addr(nv_mmap_t* mmap) ;

size_t nv_mmap_get_length(nv_mmap_t* mmap) ;




int nv_mmap_main() ;



#ifdef __cplusplus
}
#endif

#endif