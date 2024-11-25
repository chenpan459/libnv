#ifndef _NV_FORK_H_INCLUDED_
#define _NV_FORK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_base_include.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

// 定义函数指针类型
typedef void (*process_func_t)(void);

pid_t nv_create_process(process_func_t func) ;

int nv_wait_process(pid_t pid) ;
int nv_create_daemon() ;


#ifdef __cplusplus
}
#endif

#endif 