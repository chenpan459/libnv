#ifndef _NV_SHM_H_INCLUDED_
#define _NV_SHM_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 4096 // 4 KB

typedef struct {
    int shm_fd;
    void *shm_ptr;
} nv_shm_t;


nv_shm_t* nv_shm_open() ;
// 关联共享内存到进程的地址空间
void* nv_shm_map(nv_shm_t *shm_obj) ;

// 取消关联共享内存
int nv_shm_unmap(nv_shm_t *shm_obj) ;


int nv_shm_close(nv_shm_t *shm_obj) ;
// 删除共享内存对象
int nv_shm_unlink() ;



int nv_shm_main() ;


#ifdef __cplusplus
}
#endif

#endif
