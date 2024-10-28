
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NV_SHMEM_H_INCLUDED_
#define _NV_SHMEM_H_INCLUDED_


#include <nv_linux_config.h>
#include <nv_config.h>
#include <nv_core.h>



typedef struct {
    u_char      *addr;
    size_t       size;
    u_char*    name;
    nv_log_t   *log;
    nv_uint_t   exists;   /* unsigned  exists:1;  */
} nv_shm_t;


nv_int_t nv_shm_alloc(nv_shm_t *shm);
void nv_shm_free(nv_shm_t *shm);




#endif /* _NGX_SHMEM_H_INCLUDED_ */
