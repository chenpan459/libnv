#ifndef _NV_LOCK_H_INCLUDED_
#define _NV_LOCK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_base_include.h"

// 定义互斥锁类型
typedef pthread_mutex_t nv_mutex_t;

int nv_mutex_unlock(nv_mutex_t *mutex) ;
int nv_mutex_trylock(nv_mutex_t *mutex) ;
int nv_mutex_lock(nv_mutex_t *mutex) ;

int nv_mutex_destroy(nv_mutex_t *mutex) ;
int nv_mutex_init(nv_mutex_t *mutex) ;

#ifdef __cplusplus
}
#endif

#endif 