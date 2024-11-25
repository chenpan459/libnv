#ifndef _NV_SOCKET_POLL_H_INCLUDED_
#define _NV_SOCKET_POLL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"

/********************
 初始化 poll 结构：

1、使用 nv_poll_init 函数创建并初始化一个 nv_poll_t 结构。

添加文件描述符到 poll 的集合：

2、使用 nv_poll_add_fd 函数将文件描述符添加到 poll 的监控集合中，并指定要监控的事件类型（POLLIN、POLLOUT、POLLPRI 等）。
从 poll 的集合中移除文件描述符：

3、使用 nv_poll_remove_fd 函数从 poll 的监控集合中移除文件描述符。
执行 poll 操作：

4、使用 nv_poll_wait 函数执行 poll 操作，等待文件描述符上的事件发生。
销毁 poll 结构：

5、使用 nv_poll_destroy 函数销毁 nv_poll_t 结构并释放相关资源。
*****************************/

#define NV_POLL_MAX_FD 1024

typedef struct {
    struct pollfd fds[NV_POLL_MAX_FD];
    int nfds;
} nv_poll_t;


nv_poll_t* nv_poll_init() ;
int nv_poll_add_fd(nv_poll_t *poll_obj, int fd, int events) ;
int nv_poll_remove_fd(nv_poll_t *poll_obj, int fd) ;
int nv_poll_wait(nv_poll_t *poll_obj, int timeout) ;
void nv_poll_destroy(nv_poll_t *poll_obj) ;



int nv_poll_main() ;

#ifdef __cplusplus
}
#endif

#endif
