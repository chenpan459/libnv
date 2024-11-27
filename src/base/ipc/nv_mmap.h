#ifndef _NV_MMAP_H_INCLUDED_
#define _NV_MMAP_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

// 内存映射文件结构体
typedef struct {
    int fd;
    size_t size;
    void* data;
} nv_mmap_file_t;


nv_mmap_file_t* nv_mmap_open(const char* filepath, size_t size) ;
// 读取数据
void nv_mmap_read(nv_mmap_file_t* mmap_file, size_t offset, void* buffer, size_t size) ;

// 写入数据
void nv_mmap_write(nv_mmap_file_t* mmap_file, size_t offset, const void* buffer, size_t size) ;
// 关闭内存映射文件
void nv_mmap_close(nv_mmap_file_t* mmap_file) ;




#ifdef __cplusplus
}
#endif

#endif