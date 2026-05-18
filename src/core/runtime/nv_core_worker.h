#ifndef _NV_CORE_WORKER_H_INCLUDED_
#define _NV_CORE_WORKER_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

/* 若配置 worker_processes>1：master 派生子进程并监督；worker 返回 NV_OK 继续运行 */
int nv_core_workers_launch(nv_core_ctx_t *ctx);

int nv_core_worker_is_master(const nv_core_ctx_t *ctx);
int nv_core_worker_is_worker(const nv_core_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
