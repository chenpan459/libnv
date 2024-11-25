#ifndef _NV_FILE_H_INCLUDED_
#define _NV_FILE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"
#include <stdio.h>
#include <stdlib.h>


FILE* nv_open_file(const char* filename, const char* mode) ;

// 读取文件
size_t nv_read_file(void* ptr, size_t size, size_t nmemb, FILE* file) ;
// 写入文件
size_t nv_write_file(const void* ptr, size_t size, size_t nmemb, FILE* file) ;
// 关闭文件
int nv_close_file(FILE* file) ;
// 获取文件大小
long nv_get_file_size(FILE* file) ;

// 移动文件指针
int nv_seek_file(FILE* file, long offset, int whence) ;


// 获取当前文件指针位置
long nv_tell_file(FILE* file) ;




int nv_file_main();
#ifdef __cplusplus
}
#endif

#endif