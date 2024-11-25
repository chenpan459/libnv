#ifndef _NV_SOCKET_EPOLL_H_INCLUDED_
#define _NV_SOCKET_EPOLL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"

/************************************
 * select()：这是一种传统的I/O多路复用方法，它可以同时监控多个文件描述符，以确定哪些文件描述符已经准备好进行I/O操作。
       select()会阻塞直到有文件描述符就绪或者超时

poll()：与select()类似，poll()也是一种I/O多路复用的方法，它不使用固定大小的位数组，因此没有select()中1024个文件描述符的限制。

epoll：这是Linux下效率较高的I/O多路复用技术，它只在真正发生的事件上通知，而不是像select()和poll()那样需要检查所有注册的文件描述符。
       epoll使用一组API，包括epoll_create()、epoll_ctl()和epoll_wait()来管理事件。
************************************/

/**************************************
 1、初始化 epoll 实例：

1、使用 nv_epoll_create 函数创建并初始化一个 epoll 实例。
添加文件描述符到 epoll 实例：

2、使用 nv_epoll_add 函数将文件描述符（如 socket）添加到 epoll 实例，并指定要监听的事件类型。
修改文件描述符在 epoll 实例中的事件：

3、使用 nv_epoll_mod 函数修改文件描述符在 epoll 实例中监听的事件类型。
从 epoll 实例中移除文件描述符：

4、使用 nv_epoll_del 函数从 epoll 实例中移除文件描述符。
等待事件发生：

5、使用 nv_epoll_wait 函数等待文件描述符上的事件发生。
销毁 epoll 实例：

6、使用 nv_epoll_destroy 函数销毁 epoll 实例并释放相关资源。


边缘触发(EPOLLET)和水平触发(EPOLLET)：这是epoll的两种触发模式，边缘触发是指只有当状态发生变化时才会通知，
而水平触发是指只要文件描述符就绪就会一直通知

*************************************/


#define MAX_EVENTS 10
#define NV_EPOLL_ERR -1
#define NV_EPOLL_SUCCESS 0

typedef struct {
    int epoll_fd;
    struct epoll_event *events;
} nv_epoll_t;


// 初始化 epoll 实例
nv_epoll_t* nv_epoll_create() ;
int nv_epoll_add(nv_epoll_t *epoll_obj, int fd, uint32_t events) ;
int nv_epoll_mod(nv_epoll_t *epoll_obj, int fd, uint32_t events) ;
int nv_epoll_del(nv_epoll_t *epoll_obj, int fd) ;
// 等待事件发生
int nv_epoll_wait(nv_epoll_t *epoll_obj, int timeout) ;
void nv_epoll_destroy(nv_epoll_t *epoll_obj) ;


int nv_epoll_main() ;


#ifdef __cplusplus
}
#endif

#endif