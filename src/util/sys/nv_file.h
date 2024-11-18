#ifndef _NV_FILE_H_INCLUDED_
#define _NV_FILE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"


FILE* nv_open_file(const char* filename, const char* mode);
size_t nv_read_file(void* ptr, size_t size, size_t nmemb, FILE* file);
size_t nv_write_file(const void* ptr, size_t size, size_t nmemb, FILE* file) ;
int nv_close_file(FILE* file);


int nv_file_main();
#ifdef __cplusplus
}
#endif

#endif