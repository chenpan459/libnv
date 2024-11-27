#ifndef _NV_PERMISSIONS_H_INCLUDED_
#define _NV_PERMISSIONS_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
int nv_set_file_permissions(const char* filepath, mode_t mode) ;

// 获取文件权限
int nv_get_file_permissions(const char* filepath, mode_t* mode) ;
// 设置文件所有者和组
int nv_set_file_owner(const char* filepath, const char* owner, const char* group) ;
// 获取文件所有者和组
int nv_get_file_owner(const char* filepath, char* owner, size_t owner_size, char* group, size_t group_size) ;


#ifdef __cplusplus
}
#endif

#endif 