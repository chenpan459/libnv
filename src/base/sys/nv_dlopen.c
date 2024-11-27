
#include "nv_dlopen.h"
/************************************************
 * @文件名: nv_dlopen.c
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 ***********************************************/




char *nv_dlerror(void)
{
    char  *err;

    err = (char *) dlerror();

    if (err == NULL) {
        return "";
    }

    return err;
}
