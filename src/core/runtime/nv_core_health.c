/************************************************
 * @文件名: nv_core_health.c
 * @功能: 守护进程运行时健康检查实现
 ***********************************************/

#include "nv_core_health.h"

#include <nv_event.h>
#include <nv_log.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

static int g_systemd_notify = 0;

int nv_core_systemd_enabled(void)
{
    return g_systemd_notify;
}

int nv_core_systemd_notify(const char *state)
{
    const char *sock;
    struct sockaddr_un addr;
    int          fd;
    size_t       len;

    if (!state || !g_systemd_notify) {
        return NV_OK;
    }

    sock = getenv("NOTIFY_SOCKET");
    if (!sock || !sock[0]) {
        return NV_OK;
    }

    fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        return NV_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    if (sock[0] == '@') {
        /* abstract namespace socket */
        addr.sun_path[0] = '\0';
        strncpy(addr.sun_path + 1, sock + 1, sizeof(addr.sun_path) - 2);
        len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(sock + 1);
    } else {
        strncpy(addr.sun_path, sock, sizeof(addr.sun_path) - 1);
        len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);
    }

    if (sendto(fd, state, strlen(state), MSG_NOSIGNAL,
               (struct sockaddr *)&addr, (socklen_t)len) < 0) {
        close(fd);
        return NV_ERROR;
    }

    close(fd);
    return NV_OK;
}

void nv_core_get_runtime_stats(nv_core_ctx_t *ctx, nv_core_runtime_stats_t *out)
{
    nv_loop_stats_t lst;

    if (!out) {
        return;
    }

    memset(out, 0, sizeof(*out));
    if (!ctx) {
        return;
    }

    out->started_at = ctx->started_at;
    if (ctx->started_at > 0) {
        out->uptime_sec = (unsigned long)(time(NULL) - ctx->started_at);
    }
    out->quitting  = ctx->quit ? 1 : 0;
    out->reloading = ctx->reload ? 1 : 0;

    memset(&lst, 0, sizeof(lst));
    nv_loop_get_stats(&ctx->loop, &lst);
    out->loop_events = lst.events_processed;
    out->loop_timers = lst.timers_processed;
    out->loop_idle   = lst.idle_events_processed;
}

static void nv_core_heartbeat_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_core_ctx_t       *ctx = (nv_core_ctx_t *)data;
    nv_event_ext_t      *tev = (nv_event_ext_t *)ev;
    nv_core_runtime_stats_t st;

    (void)loop;

    if (!ctx || !tev) {
        return;
    }

    nv_core_get_runtime_stats(ctx, &st);
    nv_log_debug("heartbeat uptime=%lus events=%lu timers=%lu idle=%lu",
                 st.uptime_sec, st.loop_events, st.loop_timers, st.loop_idle);

    if (g_systemd_notify) {
        nv_core_systemd_notify("WATCHDOG=1");
    }

    if (ctx->opts.heartbeat_interval_sec > 0) {
        unsigned long ms = (unsigned long)ctx->opts.heartbeat_interval_sec * 1000UL;
        nv_loop_add_timer(&ctx->loop, tev, ms);
    }
}

int nv_core_health_init(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }

    ctx->started_at = time(NULL);

    if (ctx->opts.systemd_notify == NV_CORE_SYSTEMD_AUTO) {
        g_systemd_notify = (getenv("NOTIFY_SOCKET") != NULL) ? 1 : 0;
    } else {
        g_systemd_notify = ctx->opts.systemd_notify ? 1 : 0;
    }

    if (g_systemd_notify) {
        nv_core_systemd_notify("READY=1");
        nv_log_info("systemd notify enabled (READY)");
    }

    if (ctx->opts.heartbeat_interval_sec > 0) {
        unsigned long ms = (unsigned long)ctx->opts.heartbeat_interval_sec * 1000UL;

        memset(&ctx->heartbeat_ev, 0, sizeof(ctx->heartbeat_ev));
        ctx->heartbeat_ev.type    = NV_EVENT_TYPE_TIMER;
        ctx->heartbeat_ev.handler = nv_core_heartbeat_handler;
        ctx->heartbeat_ev.data    = ctx;

        if (nv_loop_add_timer(&ctx->loop, &ctx->heartbeat_ev, ms) != NV_OK) {
            nv_log_warning("heartbeat timer register failed");
        } else {
            nv_log_info("heartbeat every %d sec", ctx->opts.heartbeat_interval_sec);
        }
    }

    ctx->health_inited = 1;
    return NV_OK;
}

void nv_core_health_cleanup(nv_core_ctx_t *ctx)
{
    if (!ctx || !ctx->health_inited) {
        return;
    }

    if (g_systemd_notify) {
        nv_core_systemd_notify("STOPPING=1");
    }

    if (ctx->opts.heartbeat_interval_sec > 0) {
        nv_loop_del_timer(&ctx->loop, &ctx->heartbeat_ev);
    }

    ctx->health_inited = 0;
    g_systemd_notify   = 0;
}
