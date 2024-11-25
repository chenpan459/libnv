
#ifndef _NV_SINGNAL_H_INCLUDED_
#define _NV_SINGNAL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"
// 定义信号处理函数类型
typedef void (*nv_signal_handler_t)(int);

// 注册信号处理函数
int nv_signal_register(int signum, nv_signal_handler_t handler) ;
int nv_signal_send(pid_t pid, int signum) ;

// 示例信号处理函数
void signal_handler(int signum) ;

#ifdef __cplusplus
}
#endif

#endif
