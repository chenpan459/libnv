#include "nv_core_modules.h"

#include <nv_log.h>
#include <nv_tcp.h>
#include <nv_udp.h>

#include <string.h>

/* ---------- 事件模块：周期 tick，供业务挂接 ---------- */
static nv_event_ext_t g_mod_tick_ev;

static void nv_mod_event_tick(nv_loop_t *loop, void *ev, void *data)
{
    nv_core_ctx_t *ctx = (nv_core_ctx_t *)data;
    nv_event_ext_t *tev = (nv_event_ext_t *)ev;

    (void)loop;
    if (!ctx || !tev) {
        return;
    }

  nv_log_debug("event module tick (worker=%d)", ctx->worker_id);
    nv_loop_add_timer(&ctx->loop, tev, 5000);
}

/* core 阶段占位（loop 尚未创建） */
static int nv_mod_event_core_init(void *conf)
{
    (void)conf;
    return NV_OK;
}

static int nv_mod_event_core_cleanup(void *ctx)
{
    (void)ctx;
    return NV_OK;
}

NV_MODULE_REGISTER(NV_MODULE_EVENT, main_event, nv_mod_event_core_init,
                   nv_mod_event_core_cleanup);

static nv_tcp_pool_t g_tcp_pool;
static nv_udp_pool_t g_udp_pool;

static int nv_mod_network_core_init(void *conf)
{
    (void)conf;
    return NV_OK;
}

static int nv_mod_network_core_cleanup(void *ctx)
{
    (void)ctx;
    return NV_OK;
}

NV_MODULE_REGISTER(NV_MODULE_NETWORK, main_network, nv_mod_network_core_init,
                   nv_mod_network_core_cleanup);

static int nv_mod_event_loop_init(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }

    memset(&g_mod_tick_ev, 0, sizeof(g_mod_tick_ev));
    g_mod_tick_ev.type    = NV_EVENT_TYPE_TIMER;
    g_mod_tick_ev.handler = nv_mod_event_tick;
    g_mod_tick_ev.data    = ctx;

    if (nv_loop_add_timer(&ctx->loop, &g_mod_tick_ev, 5000) != 0) {
        nv_log_warning("event module: tick timer failed");
        return NV_ERROR;
    }

    nv_log_info("event module: 5s tick timer on loop");
    return NV_OK;
}

static void nv_mod_event_loop_cleanup(nv_core_ctx_t *ctx)
{
    if (ctx) {
        nv_loop_del_timer(&ctx->loop, &g_mod_tick_ev);
    }
}

static int nv_mod_network_loop_init(nv_core_ctx_t *ctx)
{
    int maxc;

    if (!ctx) {
        return NV_ERROR;
    }

    maxc = ctx->opts.worker_connections;
    if (maxc < 16) {
        maxc = 16;
    }

    if (nv_tcp_pool_create(&g_tcp_pool, &ctx->loop, maxc) != 0) {
        nv_log_warning("network module: tcp pool skipped");
    }
    if (nv_udp_pool_create(&g_udp_pool, &ctx->loop, maxc) != 0) {
        nv_log_warning("network module: udp pool skipped");
    }

    nv_log_info("network module: tcp/udp pools ready (max=%d)", maxc);
    return NV_OK;
}

static void nv_mod_network_loop_cleanup(void)
{
    nv_tcp_pool_destroy(&g_tcp_pool);
    nv_udp_pool_destroy(&g_udp_pool);
}

int nv_core_modules_init(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }

    if (nv_mod_event_loop_init(ctx) != NV_OK) {
        return NV_ERROR;
    }
    if (nv_mod_network_loop_init(ctx) != NV_OK) {
        nv_mod_event_loop_cleanup(ctx);
        return NV_ERROR;
    }
    return NV_OK;
}

void nv_core_modules_cleanup(nv_core_ctx_t *ctx)
{
    nv_mod_network_loop_cleanup();
    nv_mod_event_loop_cleanup(ctx);
}
