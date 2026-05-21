/************************************************
 * @文件名: nv_core_health.c
 * @功能: 守护进程运行时健康检查实现
 ***********************************************/

#include "nv_core_health.h"

#include <nv_event.h>
#include <nv_log.h>
#include <nv_watchdog.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
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

    nv_core_feed_watchdog(ctx);
    nv_core_publish_metrics(ctx);

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

    if (ctx->opts.watchdog_enable) {
        if (ctx->opts.watchdog_device && ctx->opts.watchdog_device[0]) {
            if (nv_watchdog_open(ctx->opts.watchdog_device) != NV_OK) {
                nv_log_warning("watchdog open failed: %s", ctx->opts.watchdog_device);
            }
        }
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

    nv_watchdog_close();
    ctx->health_inited = 0;
    g_systemd_notify   = 0;
}

int nv_core_feed_watchdog(nv_core_ctx_t *ctx)
{
    if (!ctx || !ctx->opts.watchdog_enable) {
        return NV_DECLINED;
    }
    if (ctx->opts.watchdog_cmd && ctx->opts.watchdog_cmd[0]) {
        return nv_watchdog_feed_cmd(ctx->opts.watchdog_cmd);
    }
    return nv_watchdog_feed();
}

int nv_core_publish_metrics(nv_core_ctx_t *ctx)
{
    nv_core_runtime_stats_t st;
    nv_log_queue_stats_t    lqs;
    char                    buf[640];
    int                     n;

    if (!ctx || !ctx->opts.metrics_publish || !ctx->pubsub_inited) {
        return NV_DECLINED;
    }
    if (!ctx->opts.metrics_topic || !ctx->opts.metrics_topic[0]) {
        return NV_DECLINED;
    }

    nv_core_get_runtime_stats(ctx, &st);
    nv_log_get_queue_stats(&lqs);

    n = snprintf(buf, sizeof(buf),
                 "{\"uptime\":%lu,\"loop_events\":%lu,\"loop_timers\":%lu,"
                 "\"log_pending\":%zu,\"log_dropped\":%llu,\"quitting\":%d}",
                 st.uptime_sec, st.loop_events, st.loop_timers,
                 lqs.pending, (unsigned long long)lqs.dropped, st.quitting);
    if (n <= 0 || (size_t)n >= sizeof(buf)) {
        return NV_ERROR;
    }
    return nv_core_pubsub_publish(ctx, ctx->opts.metrics_topic, buf, (size_t)n);
}

void nv_core_write_tombstone(nv_core_ctx_t *ctx, int signum)
{
    FILE                   *fp;
    nv_core_runtime_stats_t st;
    const char             *path;

    if (!ctx) {
        return;
    }
    path = ctx->opts.tombstone_file;
    if (!path || !path[0]) {
        path = NV_CORE_DEFAULT_TOMBSTONE_FILE;
    }

    fp = fopen(path, "a");
    if (!fp) {
        return;
    }

    nv_core_get_runtime_stats(ctx, &st);
    fprintf(fp,
            "--- nv tombstone pid=%d signum=%d (%s) phase=%s uptime=%lu ---\n",
            (int)getpid(), signum, strsignal(signum),
            nv_core_phase_name(ctx->phase), st.uptime_sec);
    fclose(fp);
}
