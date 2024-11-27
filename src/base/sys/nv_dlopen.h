
/************************************************
 * @文件名: nv_socket_types.h
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本数据类型
 ***********************************************/

#ifndef _NV_DLOPEN_H_INCLUDED_
#define _NV_DLOPEN_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <dlfcn.h>


#define nv_dlopen(path)           dlopen((char *) path, RTLD_NOW | RTLD_GLOBAL)
#define nv_dlopen_n               "dlopen()"

#define nv_dlsym(handle, symbol)  dlsym(handle, symbol)
#define nv_dlsym_n                "dlsym()"

#define nv_dlclose(handle)        dlclose(handle)
#define nv_dlclose_n              "dlclose()"



char *nv_dlerror(void);



#ifdef __cplusplus
}
#endif

#endif /* _NGX_DLOPEN_H_INCLUDED_ */
