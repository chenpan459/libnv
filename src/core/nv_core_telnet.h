#ifndef _NV_CORE_TELNET_H_INCLUDED_
#define _NV_CORE_TELNET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

#define NV_CORE_DEFAULT_TELNET_PORT  2323

int  nv_core_telnet_init(nv_core_ctx_t *ctx);
void nv_core_telnet_cleanup(nv_core_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
