
#ifndef _NV_SINGNAL_H_INCLUDED_
#define _NV_SINGNAL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <signal.h>

typedef void (*nv_signal_handler_t)(int signum);

/* 初始化 signalfd（阻塞信号并创建 fd，供 epoll 读取） */
int nv_signal_init(void);
void nv_signal_shutdown(void);

/* 注册信号处理（线程安全：仅在 init 后、dispatch 前调用） */
int nv_signal_register(int signum, nv_signal_handler_t handler);

/* signalfd 文件描述符，挂到 epoll */
int nv_signal_fd(void);

/* 从 signalfd 读取并分发（在 epoll 回调中调用） */
void nv_signal_dispatch(int signalfd_fd);

int nv_signal_send(pid_t pid, int signum);

void signal_handler(int signum);

#ifdef __cplusplus
}
#endif

#endif
