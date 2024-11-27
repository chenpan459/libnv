
#ifndef _NV_SOCKET_SELECT_H_INCLUDED_
#define _NV_SOCKET_SELECT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>


/*******************************
 * 初始化 select 结构：

1、使用 nv_select_init 函数创建并初始化一个 nv_select_t 结构。
添加文件描述符到 select 的集合：

2、使用 nv_select_add_fd 函数将文件描述符添加到 select 的监控集合中，并指定要监控的事件类型（读、写、异常）。
从 select 的集合中移除文件描述符：

3、使用 nv_select_remove_fd 函数从 select 的监控集合中移除文件描述符。
设置 select 的超时时间：

4、使用 nv_select_set_timeout 函数设置 select 调用的超时时间。
执行 select 操作：

5、使用 nv_select_wait 函数执行 select 操作，等待文件描述符上的事件发生。
销毁 select 结构：

6、使用 nv_select_destroy 函数销毁 nv_select_t 结构并释放相关资源。
*************************************/
#define NV_SELECT_MAX_FD 1024

typedef struct {
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;
    int max_fd;
    struct timeval timeout;
} nv_select_t;


nv_select_t* nv_select_init() ;
int nv_select_add_fd(nv_select_t *select_obj, int fd, int read, int write, int except) ;
int nv_select_remove_fd(nv_select_t *select_obj, int fd) ;
int nv_select_set_timeout(nv_select_t *select_obj, long sec, long usec) ;
int nv_select_wait(nv_select_t *select_obj) ;
void nv_select_destroy(nv_select_t *select_obj) ;



int nv_select_main() ;

#ifdef __cplusplus
}
#endif

#endif
