#ifndef _NV_FIFO_H_INCLUDED_
#define _NV_FIFO_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>

typedef struct {
    int fd;
} fifo_t;

fifo_t* nv_fifo_create(const char* name);
fifo_t* nv_fifo_open(fifo_t* fifo, const char* name, int mode, int nonblock);
ssize_t nv_fifo_write(fifo_t* fifo, const void* buf, size_t count);
ssize_t nv_fifo_read(fifo_t* fifo, void* buf, size_t count);

/* 新增：带超时的读写（毫秒；<0表示阻塞） */
ssize_t nv_fifo_write_timeout(fifo_t* fifo, const void* buf, size_t count, int timeout_ms);
ssize_t nv_fifo_read_timeout(fifo_t* fifo, void* buf, size_t count, int timeout_ms);

void nv_fifo_close(fifo_t* fifo);
void nv_fifo_unlink(const char* name);

int nv_fifo_main();

#ifdef __cplusplus
}
#endif

#endif