#ifndef _NV_CMD_H_INCLUDED_
#define _NV_CMD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"


int nv_system(const char* command) ;

// 示例：使用封装的 nv_system 函数执行 mkfs 和 fsck 命令
int nv_mkfs(const char* filesystem_type, const char* device) ;

int nv_fsck(const char* device) ;


#ifdef __cplusplus
}
#endif

#endif