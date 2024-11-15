#ifndef _NV_PIPE_H_INCLUDED_
#define _NV_PIPE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "nv_util_include.h"



typedef struct {
    int fd[2];
} nv_pipe_t;

nv_pipe_t* nv_pipe_create() ;
ssize_t nv_pipe_write(nv_pipe_t* pipe, const void* buf, size_t count) ;
ssize_t nv_pipe_read(nv_pipe_t* pipe, void* buf, size_t count) ;
void nv_pipe_close(nv_pipe_t* pipe) ;




int nv_pipe_main();
#ifdef __cplusplus
}
#endif

#endif 