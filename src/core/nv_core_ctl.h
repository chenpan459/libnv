#ifndef _NV_CORE_CTL_H_INCLUDED_
#define _NV_CORE_CTL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

#define NV_CORE_DEFAULT_CTL_SOCKET "/var/run/nv.ctl.sock"

int  nv_core_ctl_init(nv_core_ctx_t *ctx);
void nv_core_ctl_cleanup(nv_core_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
