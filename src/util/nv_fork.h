#ifndef _NV_FORK_H_INCLUDED_
#define _NV_FORK_H_INCLUDED_

#include "nv_util_include.h"


// 定义函数指针类型
typedef void (*process_func_t)(void);

pid_t nv_create_process(process_func_t func) ;

int nv_wait_process(pid_t pid) ;
int nv_create_daemon() ;

#endif 