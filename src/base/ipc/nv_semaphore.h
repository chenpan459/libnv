#ifndef _NV_SEMAPHORE_H_INCLUDED_
#define _NV_SEMAPHORE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>

#define SEM_NAME "/my_semaphore"

typedef struct {
    sem_t *sem;
} nv_semaphore_t;

nv_semaphore_t* nv_semaphore_open(const char* name, unsigned int value);
int nv_semaphore_close(nv_semaphore_t* sem_obj) ;

int nv_semaphore_unlink(const char* name) ;
int nv_semaphore_wait(nv_semaphore_t* sem_obj) ;
int nv_semaphore_post(nv_semaphore_t* sem_obj) ;


int nv_semaphore_main() ;

#ifdef __cplusplus
}
#endif

#endif
