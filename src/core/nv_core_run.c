/************************************************
 * nv_core_run.c - 主循环、退出、统一入口
 ***********************************************/
#include "nv_core_private.h"
#include "nv_core_config.h"
#include "nv_core_health.h"
#include "nv_core_lock.h"
#include "nv_core_worker.h"
#include "nv_version.h"
#include <nv_log.h>
#include <nv_base.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>

nv_core_ctx_t        *g_core_ctx    = NULL;
const nv_core_hooks_t *g_core_hooks  = NULL;
nv_event_ext_t        g_idle_ev;
char                **g_saved_argv   = NULL;
int                   g_saved_argc   = 0;

void nv_core_check_shutdown_timeout(nv_core_ctx_t *ctx)
{
    time_t elapsed;

    if (!ctx || !ctx->quit || ctx->opts.shutdown_timeout_sec <= 0) {
        return;
    }
    if (!ctx->shutdown_started_at) {
        ctx->shutdown_started_at = time(NULL);
    }

    elapsed = time(NULL) - ctx->shutdown_started_at;
    if (elapsed >= (time_t)ctx->opts.shutdown_timeout_sec) {
        nv_log_error("shutdown timeout (%d sec), force exit",
                     ctx->opts.shutdown_timeout_sec);
        _exit(1);
    }
}

int nv_core_request_quit(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }
    ctx->quit = 1;
    if (!ctx->shutdown_started_at) {
        ctx->shutdown_started_at = time(NULL);
    }
    nv_loop_stop(&ctx->loop);
    return NV_OK;
}

int nv_core_is_quitting(const nv_core_ctx_t *ctx)
{
    return ctx && ctx->quit;
}

void nv_core_idle_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_core_ctx_t *ctx = (nv_core_ctx_t *)data;
    (void)loop;
    (void)ev;

    if (!ctx) {
        return;
    }

    nv_core_check_shutdown_timeout(ctx);

    if (ctx->reload) {
        ctx->reload = 0;
        nv_log_info("reload config (SIGHUP)");
        if (nv_core_reload_config(ctx) == NV_OK &&
            g_core_hooks && g_core_hooks->on_reload) {
            g_core_hooks->on_reload(ctx);
        }
    }

    if (ctx->fatal_signum) {
        nv_loop_stop(&ctx->loop);
        return;
    }

    if (g_core_hooks && g_core_hooks->on_idle) {
        g_core_hooks->on_idle(ctx);
    }
}

int nv_core_run_loop(nv_core_ctx_t *ctx, const nv_core_hooks_t *hooks)
{
    int rc;

    if (!ctx) {
        return NV_ERROR;
    }

    (void)hooks;
    ctx->phase = NV_CORE_PHASE_LOOP;
    nv_log_info("enter main event loop [%s]", nv_core_phase_name(ctx->phase));

    memset(&g_idle_ev, 0, sizeof(g_idle_ev));
    g_idle_ev.type    = NV_EVENT_TYPE_IDLE;
    g_idle_ev.handler = nv_core_idle_handler;
    g_idle_ev.data    = ctx;
    nv_loop_add_idle(&ctx->loop, &g_idle_ev);

    if (nv_core_health_init(ctx) != NV_OK) {
        nv_log_warning("health module init partial failure");
    }

    rc = nv_loop_run(&ctx->loop);

    if (ctx->fatal_signum) {
        nv_core_handle_fatal(ctx);
        return NV_ERROR;
    }
    return rc;
}

void nv_core_shutdown(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    if (ctx->phase == NV_CORE_PHASE_SHUTDOWN) {
        return;
    }

    ctx->phase = NV_CORE_PHASE_SHUTDOWN;
    nv_log_info("graceful shutdown [%s]", nv_core_phase_name(ctx->phase));

    nv_loop_stop(&ctx->loop);
    nv_base_cleanup();

    nv_core_pidfile_remove(ctx);
    nv_core_instance_lock_release(ctx->instance_lock_fd, ctx->opts.instance_lock);
    ctx->instance_lock_fd = -1;
    nv_log_close();
    if (ctx->opts.use_syslog) {
        closelog();
    }

    nv_ini_free(ctx->ini);
    ctx->ini = NULL;
    free(ctx->pid_file_dup);
    free(ctx->log_file_dup);
    free(ctx->mq_name_dup);
    free(ctx->ctl_socket_dup);
    free(ctx->instance_lock_dup);
    free(ctx->telnet_bind_dup);
    free(ctx->cli_username_dup);
    free(ctx->cli_password_dup);
    ctx->pid_file_dup = NULL;
    ctx->log_file_dup = NULL;
    ctx->mq_name_dup  = NULL;
    ctx->ctl_socket_dup = NULL;
    ctx->instance_lock_dup = NULL;
    ctx->telnet_bind_dup = NULL;
    ctx->cli_username_dup = NULL;
    ctx->cli_password_dup = NULL;

    g_core_ctx = NULL;
    ctx->phase = NV_CORE_PHASE_NONE;
}

void nv_core_free_saved_argv(void)
{
    int i;

    for (i = 0; i < g_saved_argc; i++) {
        free(g_saved_argv[i]);
    }
    free(g_saved_argv);
    g_saved_argv = NULL;
    g_saved_argc = 0;
}

void nv_core_save_argv(int argc, char **argv)
{
    int i;

    for (i = 0; i < g_saved_argc; i++) {
        free(g_saved_argv[i]);
    }
    free(g_saved_argv);
    g_saved_argv = NULL;
    g_saved_argc = 0;

    if (argc <= 0 || !argv) {
        return;
    }

    g_saved_argv = calloc((size_t)argc + 1, sizeof(char *));
    if (!g_saved_argv) {
        return;
    }

    for (i = 0; i < argc; i++) {
        g_saved_argv[i] = argv[i] ? strdup(argv[i]) : NULL;
    }
    g_saved_argv[argc] = NULL;
    g_saved_argc       = argc;
}

int nv_core_run(nv_core_ctx_t *ctx, int argc, char **argv,
                const nv_core_hooks_t *hooks)
{
    int rc;

    if (!ctx) {
        return NV_ERROR;
    }

    g_core_hooks = hooks;
    nv_core_save_argv(argc, argv);

    rc = nv_core_parse_args(ctx, argc, argv);
    if (rc == NV_DECLINED) {
        nv_core_free_saved_argv();
        return 0;
    }
    if (rc != NV_OK) {
        nv_core_free_saved_argv();
        return NV_ERROR;
    }

    if (nv_core_startup_init(ctx) != NV_OK) {
        nv_core_shutdown(ctx);
        nv_core_free_saved_argv();
        return NV_ERROR;
    }

    rc = nv_core_workers_launch(ctx);
    if (rc == NV_DECLINED) {
        nv_core_shutdown(ctx);
        nv_core_free_saved_argv();
        return NV_OK;
    }
    if (rc != NV_OK) {
        nv_core_shutdown(ctx);
        nv_core_free_saved_argv();
        return NV_ERROR;
    }

    if (hooks && hooks->on_business_init) {
        if (hooks->on_business_init(ctx) != NV_OK) {
            nv_log_error("user business init failed");
            nv_core_business_cleanup(ctx);
            nv_core_shutdown(ctx);
            nv_core_free_saved_argv();
            return NV_ERROR;
        }
    } else if (nv_core_business_init(ctx) != NV_OK) {
        nv_core_business_cleanup(ctx);
        nv_core_shutdown(ctx);
        nv_core_free_saved_argv();
        return NV_ERROR;
    }

    nv_core_run_loop(ctx, hooks);

    if (hooks && hooks->on_business_cleanup) {
        hooks->on_business_cleanup(ctx);
    } else {
        nv_core_business_cleanup(ctx);
    }

    if (ctx->restart) {
        char *const *exec_argv;

        nv_log_info("restart requested (SIGUSR1), re-exec");
        nv_core_shutdown(ctx);
        exec_argv = (char *const *)(g_saved_argv ? g_saved_argv : argv);
        if (exec_argv && exec_argv[0]) {
            execv(exec_argv[0], exec_argv);
        }
        nv_core_free_saved_argv();
        return NV_ERROR;
    }

    nv_log_info("main process exiting");
    nv_core_shutdown(ctx);
    nv_core_free_saved_argv();
    return NV_OK;
}
