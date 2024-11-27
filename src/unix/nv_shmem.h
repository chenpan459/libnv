
/************************************************
 * @文件名: nv_socket_types.h
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本数据类型
 ***********************************************/


#ifndef _NV_SHMEM_H_INCLUDED_
#define _NV_SHMEM_H_INCLUDED_



#include <nv_core.h>

#if 0


typedef struct {
    u_char      *addr;
    size_t       size;
    u_char*    name;
    nv_log_t   *log;
    nv_uint_t   exists;   /* unsigned  exists:1;  */
} nv_shm_t;


nv_int_t nv_shm_alloc(nv_shm_t *shm);
void nv_shm_free(nv_shm_t *shm);


#endif

#endif /* _NGX_SHMEM_H_INCLUDED_ */
