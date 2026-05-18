/************************************************
 * @文件名: nv_core.c
 * @功能: libnv 主进程框架 — 业务初始化与运行时控制
 ***********************************************/

#include "nv_core.h"
#include "nv_core_private.h"
#include "nv_core_cli.h"
#include "nv_core_health.h"
#include "nv_core_ctl.h"
#include "nv_core_telnet.h"
#include "nv_core_modules.h"

#include <nv_log.h>
#include <nv_base.h>
#include <nv_signal.h>
#include <nv_loop.h>
#include <nv_thread_pool.h>
#include <nv_message_queue.h>

#include <string.h>

int nv_core_business_init(nv_core_ctx_t *ctx)
{
    nv_loop_config_t lcfg = NV_LOOP_CONFIG_DEFAULT;

    if (!ctx) {
        return NV_ERROR;
    }

    ctx->phase = NV_CORE_PHASE_BUSINESS;
    nv_log_info("business modules init [%s]", nv_core_phase_name(ctx->phase));

    lcfg.max_events = ctx->opts.worker_connections;
    lcfg.timeout_ms = 1000;
    lcfg.use_epoll  = 1;
    lcfg.enable_timers  = 1;
    lcfg.enable_signals = 1;
    lcfg.enable_idle    = 1;

    if (nv_loop_init(&ctx->loop, &lcfg) != NV_OK) {
        nv_log_error("event loop init failed");
        return NV_ERROR;
    }

    if (nv_core_signals_register_loop(ctx) != NV_OK) {
        nv_log_error("signalfd register failed");
        return NV_ERROR;
    }

    ctx->thread_pool = nv_threadpool_create(ctx->opts.worker_threads, 256);
    if (!ctx->thread_pool) {
        nv_log_error("thread pool create failed");
        return NV_ERROR;
    }

    ctx->msg_queue = nv_init_message_queue(ctx->opts.mq_name, 10, MAX_MSG_SIZE, 0);
    if (!ctx->msg_queue) {
        nv_log_warning("message queue init skipped: %s", ctx->opts.mq_name);
    }

    if (nv_core_modules_init(ctx) != NV_OK) {
        nv_log_error("business modules init failed");
        return NV_ERROR;
    }

    nv_core_cli_init();

    if (!ctx->is_master &&
        (ctx->opts.worker_processes <= 1 || ctx->worker_id == 1)) {
        if (nv_core_ctl_init(ctx) != NV_OK) {
            nv_log_warning("control socket init failed");
        }
        if (nv_core_telnet_init(ctx) != NV_OK) {
            nv_log_warning("telnet CLI init failed");
        }
    }

    return NV_OK;
}

void nv_core_business_cleanup(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    nv_log_info("business modules cleanup");

    nv_core_health_cleanup(ctx);
    nv_core_telnet_cleanup(ctx);
    nv_core_ctl_cleanup(ctx);
    nv_core_cli_cleanup();
    nv_core_modules_cleanup(ctx);
    nv_signal_shutdown();

    if (ctx->msg_queue) {
        nv_destroy_message_queue(ctx->msg_queue);
        ctx->msg_queue = NULL;
    }

    if (ctx->thread_pool) {
        nv_threadpool_destroy(ctx->thread_pool, NV_TP_SHUTDOWN_GRACEFUL);
        ctx->thread_pool = NULL;
    }

    nv_loop_cleanup(&ctx->loop);
}
