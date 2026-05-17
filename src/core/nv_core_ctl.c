#include "nv_core_ctl.h"
#include "nv_core_cli.h"

#include <nv_log.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define NV_CORE_CTL_BACKLOG 8
#define NV_CORE_CTL_BUF     512

static void nv_core_ctl_handle_client(nv_core_ctx_t *ctx, int cfd)
{
    char buf[NV_CORE_CTL_BUF];
    ssize_t n;

    n = read(cfd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        return;
    }
    buf[n] = '\0';

    nv_core_cli_execute_line(ctx, cfd, buf);
}

static void nv_core_ctl_accept_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_core_ctx_t  *ctx = (nv_core_ctx_t *)data;
    nv_event_ext_t *cev = (nv_event_ext_t *)ev;
    int             cfd;

    (void)loop;

    if (!ctx || !cev || cev->fd < 0) {
        return;
    }

    for (;;) {
        cfd = accept(cev->fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                break;
            }
            break;
        }
        nv_core_ctl_handle_client(ctx, cfd);
        close(cfd);
    }
}

int nv_core_ctl_init(nv_core_ctx_t *ctx)
{
    struct sockaddr_un addr;
    int                sfd;

    if (!ctx || !ctx->opts.ctl_socket || !ctx->opts.ctl_socket[0]) {
        return NV_OK;
    }

    if (ctx->ctl_inited) {
        return NV_OK;
    }

    unlink(ctx->opts.ctl_socket);

    sfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sfd < 0) {
        nv_log_error("ctl socket create failed: %s", strerror(errno));
        return NV_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ctx->opts.ctl_socket, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        nv_log_error("ctl bind %s failed: %s", ctx->opts.ctl_socket, strerror(errno));
        close(sfd);
        return NV_ERROR;
    }

    if (listen(sfd, NV_CORE_CTL_BACKLOG) < 0) {
        close(sfd);
        return NV_ERROR;
    }

    memset(&ctx->ctl_listen_ev, 0, sizeof(ctx->ctl_listen_ev));
    ctx->ctl_listen_ev.fd      = sfd;
    ctx->ctl_listen_ev.type    = NV_EVENT_TYPE_IO;
    ctx->ctl_listen_ev.handler = nv_core_ctl_accept_handler;
    ctx->ctl_listen_ev.data    = ctx;
    ctx->ctl_listen_fd         = sfd;

    if (nv_loop_add_event(&ctx->loop, &ctx->ctl_listen_ev, EPOLLIN) != 0) {
        close(sfd);
        ctx->ctl_listen_fd = -1;
        return NV_ERROR;
    }

    ctx->ctl_inited = 1;
    nv_log_info("control socket listening: %s", ctx->opts.ctl_socket);
    return NV_OK;
}

void nv_core_ctl_cleanup(nv_core_ctx_t *ctx)
{
    if (!ctx || !ctx->ctl_inited) {
        return;
    }

    if (ctx->ctl_listen_fd >= 0) {
        nv_loop_del_event(&ctx->loop, &ctx->ctl_listen_ev);
        close(ctx->ctl_listen_fd);
        ctx->ctl_listen_fd = -1;
    }

    if (ctx->opts.ctl_socket) {
        unlink(ctx->opts.ctl_socket);
    }

    ctx->ctl_inited = 0;
}
