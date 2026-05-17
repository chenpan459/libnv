#ifndef _NV_CORE_CONFIG_H_INCLUDED_
#define _NV_CORE_CONFIG_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

int nv_core_config_validate(nv_core_ctx_t *ctx);
int nv_core_reload_config(nv_core_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
