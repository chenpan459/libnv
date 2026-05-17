#include "nv_core_worker.h"

#include <nv_log.h>
#include <nv_signal.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "nv_core_private.h"

#define NV_CORE_WORKER_MAX 128

typedef struct {
    pid_t pid;
    int   id;
    int   alive;
} nv_worker_slot_t;

static nv_worker_slot_t g_workers[NV_CORE_WORKER_MAX];
static int              g_worker_count;
static volatile sig_atomic_t g_master_quit;

static void nv_core_master_on_signal(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        g_master_quit = 1;
    }
}

static void nv_core_worker_reap_all(void)
{
    pid_t pid;
    int   status;
    int   i;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (i = 0; i < g_worker_count; i++) {
            if (g_workers[i].alive && g_workers[i].pid == pid) {
                g_workers[i].alive = 0;
                nv_log_warning("worker %d (pid %d) exited", g_workers[i].id, (int)pid);
                break;
            }
        }
    }
}

static int nv_core_master_supervise(nv_core_ctx_t *ctx)
{
    int i;
    int alive_count;

    g_master_quit = 0;
    ctx->is_master = 1;
    ctx->phase     = NV_CORE_PHASE_LOOP;

    nv_signal_init();
    nv_signal_register(SIGINT,  nv_core_master_on_signal);
    nv_signal_register(SIGTERM, nv_core_master_on_signal);
    nv_signal_register(SIGCHLD, nv_core_master_on_signal);

    nv_log_info("master supervising %d workers", g_worker_count);

    while (!g_master_quit) {
        nv_core_worker_reap_all();

        if (ctx->opts.worker_respawn) {
            for (i = 0; i < g_worker_count; i++) {
                pid_t npid;

                if (g_workers[i].alive) {
                    continue;
                }

                npid = fork();
                if (npid < 0) {
                    nv_log_error("respawn worker %d failed", g_workers[i].id);
                    continue;
                }
                if (npid == 0) {
                    if (g_saved_argv && g_saved_argv[0]) {
                        execv(g_saved_argv[0], g_saved_argv);
                    }
                    _exit(1);
                }

                g_workers[i].pid   = npid;
                g_workers[i].alive = 1;
                nv_log_info("respawned worker %d pid %d", g_workers[i].id, (int)npid);
            }
        }

        alive_count = 0;
        for (i = 0; i < g_worker_count; i++) {
            if (g_workers[i].alive) {
                alive_count++;
            }
        }
        if (alive_count == 0 && !ctx->opts.worker_respawn) {
            nv_log_info("all workers exited");
            break;
        }

        usleep(200000);
    }

    nv_log_info("master stopping workers");
    for (i = 0; i < g_worker_count; i++) {
        if (g_workers[i].alive && g_workers[i].pid > 0) {
            kill(g_workers[i].pid, SIGTERM);
        }
    }

    for (i = 0; i < 30; i++) {
        nv_core_worker_reap_all();
        usleep(100000);
    }

    for (i = 0; i < g_worker_count; i++) {
        if (g_workers[i].alive && g_workers[i].pid > 0) {
            kill(g_workers[i].pid, SIGKILL);
            waitpid(g_workers[i].pid, NULL, 0);
        }
    }

    nv_signal_shutdown();
    (void)ctx;
    return NV_OK;
}

int nv_core_workers_launch(nv_core_ctx_t *ctx)
{
    int   n;
    int   i;
    pid_t pid;

    if (!ctx || ctx->opts.worker_processes <= 1) {
        ctx->is_master = 0;
        ctx->is_worker = 0;
        ctx->worker_id = 0;
        return NV_OK;
    }

    n = ctx->opts.worker_processes;
    if (n > NV_CORE_WORKER_MAX) {
        n = NV_CORE_WORKER_MAX;
    }

    memset(g_workers, 0, sizeof(g_workers));
    g_worker_count = n;

    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            nv_log_error("fork worker %d failed", i + 1);
            return NV_ERROR;
        }
        if (pid == 0) {
            ctx->worker_id = i + 1;
            ctx->is_worker = 1;
            ctx->is_master = 0;
            return NV_OK;
        }

        g_workers[i].pid   = pid;
        g_workers[i].id    = i + 1;
        g_workers[i].alive = 1;
        nv_log_info("started worker %d pid %d", i + 1, (int)pid);
    }

    ctx->is_master = 1;
    nv_core_master_supervise(ctx);
    return NV_DECLINED;
}

int nv_core_worker_is_master(const nv_core_ctx_t *ctx)
{
    return ctx && ctx->is_master;
}

int nv_core_worker_is_worker(const nv_core_ctx_t *ctx)
{
    return ctx && ctx->is_worker;
}
