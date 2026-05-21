#include "nv_core_config.h"

#include <nv_log.h>

#include <string.h>

typedef struct nv_core_opts_snapshot_s {
    int         log_level;
    int         worker_threads;
    int         worker_connections;
    int         worker_processes;
    int         heartbeat_interval_sec;
    int         shutdown_timeout_sec;
    int         use_syslog;
    int         console;
} nv_core_opts_snapshot_t;

static void nv_core_opts_snapshot_save(nv_core_ctx_t *ctx, nv_core_opts_snapshot_t *snap)
{
    snap->log_level              = ctx->opts.log_level;
    snap->worker_threads         = ctx->opts.worker_threads;
    snap->worker_connections     = ctx->opts.worker_connections;
    snap->worker_processes       = ctx->opts.worker_processes;
    snap->heartbeat_interval_sec = ctx->opts.heartbeat_interval_sec;
    snap->shutdown_timeout_sec   = ctx->opts.shutdown_timeout_sec;
    snap->use_syslog             = ctx->opts.use_syslog;
    snap->console                = nv_log_get_console();
}

static void nv_core_opts_snapshot_restore(nv_core_ctx_t *ctx,
                                          const nv_core_opts_snapshot_t *snap)
{
    ctx->opts.log_level              = snap->log_level;
    ctx->opts.worker_threads         = snap->worker_threads;
    ctx->opts.worker_connections     = snap->worker_connections;
    ctx->opts.worker_processes       = snap->worker_processes;
    ctx->opts.heartbeat_interval_sec = snap->heartbeat_interval_sec;
    ctx->opts.shutdown_timeout_sec   = snap->shutdown_timeout_sec;
    ctx->opts.use_syslog             = snap->use_syslog;
    ctx->conf.log_level              = snap->log_level;
    ctx->conf.worker_processes       = snap->worker_threads;
    ctx->conf.worker_connections     = snap->worker_connections;
    nv_log_set_level((nv_log_level_e)snap->log_level);
    nv_log_set_console(snap->console);
}

int nv_core_config_validate(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }

    if (ctx->opts.log_level < NV_LOG_LEVEL_DEBUG ||
        ctx->opts.log_level > NV_LOG_LEVEL_FATAL) {
        nv_log_error("config validate: invalid log_level %d", ctx->opts.log_level);
        return NV_ERROR;
    }

    if (ctx->opts.worker_threads < 1 || ctx->opts.worker_threads > 256) {
        nv_log_error("config validate: worker threads out of range");
        return NV_ERROR;
    }

    if (ctx->opts.worker_connections < 8 || ctx->opts.worker_connections > 65536) {
        nv_log_error("config validate: worker connections out of range");
        return NV_ERROR;
    }

    if (ctx->opts.worker_processes < 0 || ctx->opts.worker_processes > 128) {
        nv_log_error("config validate: worker processes out of range");
        return NV_ERROR;
    }

    if (ctx->opts.shutdown_timeout_sec < 0 || ctx->opts.shutdown_timeout_sec > 3600) {
        nv_log_error("config validate: shutdown_timeout out of range");
        return NV_ERROR;
    }

    if (ctx->opts.heartbeat_interval_sec < 0 || ctx->opts.heartbeat_interval_sec > 86400) {
        nv_log_error("config validate: heartbeat_interval out of range");
        return NV_ERROR;
    }

    if (ctx->opts.telnet_port < 1 || ctx->opts.telnet_port > 65535) {
        nv_log_error("config validate: telnet_port out of range");
        return NV_ERROR;
    }

    if (ctx->opts.max_open_files < 0) {
        nv_log_error("config validate: max_open_files invalid");
        return NV_ERROR;
    }

    return NV_OK;
}

int nv_core_reload_config(nv_core_ctx_t *ctx)
{
    nv_core_opts_snapshot_t snap;
    int                     rc;

    if (!ctx) {
        return NV_ERROR;
    }

    nv_core_opts_snapshot_save(ctx, &snap);

    rc = nv_core_load_config(ctx);
    if (rc != NV_OK) {
        nv_log_error("reload: load config failed, rollback");
        nv_core_opts_snapshot_restore(ctx, &snap);
        return NV_ERROR;
    }

    rc = nv_core_config_validate(ctx);
    if (rc != NV_OK) {
        nv_log_error("reload: validation failed, rollback");
        nv_core_opts_snapshot_restore(ctx, &snap);
        return NV_ERROR;
    }

    nv_log_info("config reloaded and validated");
    return NV_OK;
}
