/************************************************
 * @文件名: nv_core_health.h
 * @功能: 守护进程运行时健康检查（心跳、systemd、统计）
 ***********************************************/

#ifndef _NV_CORE_HEALTH_H_INCLUDED_
#define _NV_CORE_HEALTH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

typedef struct nv_core_runtime_stats_s {
    time_t      started_at;
    unsigned long uptime_sec;
    unsigned long loop_events;
    unsigned long loop_timers;
    unsigned long loop_idle;
    int           quitting;
    int           reloading;
} nv_core_runtime_stats_t;

int  nv_core_health_init(nv_core_ctx_t *ctx);
void nv_core_health_cleanup(nv_core_ctx_t *ctx);

int  nv_core_systemd_notify(const char *state);
int  nv_core_systemd_enabled(void);

void nv_core_get_runtime_stats(nv_core_ctx_t *ctx, nv_core_runtime_stats_t *out);

#ifdef __cplusplus
}
#endif

#endif /* _NV_CORE_HEALTH_H_INCLUDED_ */
