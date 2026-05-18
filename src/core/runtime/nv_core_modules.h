#ifndef _NV_CORE_MODULES_H_INCLUDED_
#define _NV_CORE_MODULES_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

int  nv_core_modules_init(nv_core_ctx_t *ctx);
void nv_core_modules_cleanup(nv_core_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
