#ifndef _NV_FIFO_H_INCLUDED_
#define _NV_FIFO_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "nv_util_include.h"

#define FIFO_NAME "./my_fifo"

typedef struct {
    int fd;
} fifo_t;

fifo_t* nv_fifo_create(const char* name) ;
fifo_t* nv_fifo_open(fifo_t* fifo, const char* name, int mode, int nonblock) ;
ssize_t nv_fifo_write(fifo_t* fifo, const void* buf, size_t count) ;
ssize_t nv_fifo_read(fifo_t* fifo, void* buf, size_t count) ;
void nv_fifo_close(fifo_t* fifo) ;
void nv_fifo_unlink(const char* name) ;

int nv_fifo_main() ;

#ifdef __cplusplus
}
#endif

#endif