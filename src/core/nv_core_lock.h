#ifndef _NV_CORE_LOCK_H_INCLUDED_
#define _NV_CORE_LOCK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

#define NV_CORE_DEFAULT_LOCK_NAME "libnv"

int  nv_core_instance_lock_acquire(const char *lock_name, int *lock_fd);
void nv_core_instance_lock_release(int lock_fd, const char *lock_name);

#ifdef __cplusplus
}
#endif

#endif
