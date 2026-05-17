#include "nv_core_telnet.h"
#include "nv_core_cli.h"

#include <nv_log.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define NV_TELNET_SESS_MAX   16
#define NV_TELNET_BACKLOG    8
#define NV_TELNET_USER_MAX   64
#define NV_TELNET_PASS_MAX   64
#define NV_TELNET_LINE_MAX     NV_CORE_CLI_LINE_MAX
#define NV_TELNET_HISTORY_MAX  32
#define NV_TELNET_ESC_BUF_MAX  8

typedef enum {
    NV_TELNET_ST_USER = 0,
    NV_TELNET_ST_PASS,
    NV_TELNET_ST_SHELL,
} nv_telnet_state_t;

typedef struct nv_telnet_sess_s {
    int               active;
    int               fd;
    nv_core_ctx_t    *ctx;
    nv_telnet_state_t state;
    char              user[NV_TELNET_USER_MAX];
    char              line[NV_TELNET_LINE_MAX];
    size_t            line_len;
    char              history[NV_TELNET_HISTORY_MAX][NV_TELNET_LINE_MAX];
    int               history_count;
    int               history_pos;   /* -1=编辑新行，0..count-1=浏览历史 */
    char              history_draft[NV_TELNET_LINE_MAX];
    unsigned char     esc_buf[NV_TELNET_ESC_BUF_MAX];
    int               esc_len;
    nv_event_ext_t    ev;
    struct nv_telnet_sess_s *next;
} nv_telnet_sess_t;

static nv_telnet_sess_t *g_telnet_sessions;

static void nv_telnet_str_copy(char *dst, size_t dst_size, const char *src)
{
    int max_copy;

    if (!dst || dst_size == 0) {
        return;
    }
    max_copy = (int)dst_size - 1;
    if (max_copy < 0) {
        return;
    }
    snprintf(dst, dst_size, "%.*s", max_copy, src ? src : "");
}

/* Telnet 选项协商：WILL SGA/ECHO, DO ECHO, WONT LINEMODE */
static const unsigned char g_telnet_init[] = {
    255, 251, 3,
    255, 251, 1,
    255, 253, 1,
    255, 254, 34,
};

static ssize_t nv_telnet_write_raw(int fd, const void *buf, size_t len)
{
    ssize_t n = write(fd, buf, len);
    return n;
}

static void nv_telnet_sess_free(nv_telnet_sess_t *sess)
{
    if (!sess) {
        return;
    }
    if (sess->fd >= 0) {
        close(sess->fd);
        sess->fd = -1;
    }
    sess->active = 0;
    free(sess);
}

static void nv_telnet_sess_remove(nv_telnet_sess_t *sess)
{
    nv_telnet_sess_t **pp = &g_telnet_sessions;

    while (*pp) {
        if (*pp == sess) {
            *pp = sess->next;
            break;
        }
        pp = &(*pp)->next;
    }
    if (sess->ctx && sess->ctx->loop.epoll_fd >= 0 && sess->ev.fd >= 0) {
        nv_loop_del_event(&sess->ctx->loop, &sess->ev);
    }
    nv_telnet_sess_free(sess);
}

static int nv_telnet_auth(nv_core_ctx_t *ctx, const char *user, const char *pass)
{
    const char *eu;
    const char *ep;

    if (!ctx) {
        return 0;
    }
    eu = ctx->opts.cli_username ? ctx->opts.cli_username : "admin";
    ep = ctx->opts.cli_password ? ctx->opts.cli_password : "nvadmin";

    if (!user || !pass) {
        return 0;
    }
    return (strcmp(user, eu) == 0 && strcmp(pass, ep) == 0);
}

static void nv_telnet_send_login_user(nv_telnet_sess_t *sess)
{
    nv_core_cli_print_banner(sess->fd);
    nv_core_cli_write(sess->fd, "login: ");
    sess->state = NV_TELNET_ST_USER;
}

static void nv_telnet_send_login_pass(nv_telnet_sess_t *sess)
{
    nv_core_cli_write(sess->fd, "\r\npassword: ");
    sess->state = NV_TELNET_ST_PASS;
}

static void nv_telnet_send_shell(nv_telnet_sess_t *sess)
{
    nv_core_cli_write(sess->fd, "\r\nOK login successful\r\n");
    nv_core_cli_print_prompt(sess->fd);
    sess->state         = NV_TELNET_ST_SHELL;
    sess->history_pos   = -1;
    sess->history_count = 0;
    sess->esc_len       = 0;
}

static void nv_telnet_redraw_line(nv_telnet_sess_t *sess)
{
    sess->line[sess->line_len] = '\0';
    nv_core_cli_write(sess->fd, "\r\x1b[Knv> %s", sess->line);
}

static void nv_telnet_history_add(nv_telnet_sess_t *sess, const char *line)
{
    int n;

    if (!line || !line[0] || sess->state != NV_TELNET_ST_SHELL) {
        return;
    }

    if (sess->history_count > 0 &&
        strcmp(sess->history[sess->history_count - 1], line) == 0) {
        return;
    }

    if (sess->history_count < NV_TELNET_HISTORY_MAX) {
        n = sess->history_count++;
    } else {
        memmove(sess->history[0], sess->history[1],
                (size_t)(NV_TELNET_HISTORY_MAX - 1) * NV_TELNET_LINE_MAX);
        n = NV_TELNET_HISTORY_MAX - 1;
    }

    nv_telnet_str_copy(sess->history[n], NV_TELNET_LINE_MAX, line);
    sess->history_pos = -1;
}

static void nv_telnet_history_up(nv_telnet_sess_t *sess)
{
    if (sess->state != NV_TELNET_ST_SHELL || sess->history_count <= 0) {
        return;
    }

    if (sess->history_pos < 0) {
        nv_telnet_str_copy(sess->history_draft, NV_TELNET_LINE_MAX, sess->line);
        sess->history_pos = sess->history_count - 1;
    } else if (sess->history_pos > 0) {
        sess->history_pos--;
    } else {
        return;
    }

    nv_telnet_str_copy(sess->line, NV_TELNET_LINE_MAX, sess->history[sess->history_pos]);
    sess->line_len = strlen(sess->line);
    nv_telnet_redraw_line(sess);
}

static void nv_telnet_history_down(nv_telnet_sess_t *sess)
{
    if (sess->state != NV_TELNET_ST_SHELL || sess->history_pos < 0) {
        return;
    }

    if (sess->history_pos >= sess->history_count - 1) {
        sess->history_pos = -1;
        nv_telnet_str_copy(sess->line, NV_TELNET_LINE_MAX, sess->history_draft);
    } else {
        sess->history_pos++;
        nv_telnet_str_copy(sess->line, NV_TELNET_LINE_MAX, sess->history[sess->history_pos]);
    }

    sess->line_len = strlen(sess->line);
    nv_telnet_redraw_line(sess);
}

static int nv_telnet_escape_feed(nv_telnet_sess_t *sess, unsigned char c)
{
    const char *seq;

    if (sess->esc_len == 0 && c != 0x1b) {
        return 0;
    }

    if (sess->esc_len < NV_TELNET_ESC_BUF_MAX - 1) {
        sess->esc_buf[sess->esc_len++] = c;
        sess->esc_buf[sess->esc_len]   = '\0';
    }

    seq = (const char *)sess->esc_buf;
    if (strstr(seq, "[A") != NULL || strstr(seq, "OA") != NULL) {
        nv_telnet_history_up(sess);
        sess->esc_len = 0;
        return 1;
    }
    if (strstr(seq, "[B") != NULL || strstr(seq, "OB") != NULL) {
        nv_telnet_history_down(sess);
        sess->esc_len = 0;
        return 1;
    }

    /* 未知或过长的转义序列，丢弃 */
    if (sess->esc_len >= 3 || (sess->esc_len >= 2 && seq[1] != '[' && seq[1] != 'O')) {
        sess->esc_len = 0;
    }
    return 1;
}

static void nv_telnet_process_line(nv_telnet_sess_t *sess)
{
    sess->line[sess->line_len] = '\0';

    switch (sess->state) {
    case NV_TELNET_ST_USER:
        nv_telnet_str_copy(sess->user, sizeof(sess->user), sess->line);
        sess->line_len = 0;
        nv_telnet_send_login_pass(sess);
        break;

    case NV_TELNET_ST_PASS:
        if (nv_telnet_auth(sess->ctx, sess->user, sess->line)) {
            sess->line_len = 0;
            nv_telnet_send_shell(sess);
        } else {
            nv_core_cli_write(sess->fd, "\r\nlogin incorrect\r\n");
            sess->line_len = 0;
            nv_telnet_send_login_user(sess);
        }
        break;

    case NV_TELNET_ST_SHELL:
        nv_telnet_history_add(sess, sess->line);
        nv_core_cli_write(sess->fd, "\r\n");
        nv_core_cli_execute_line(sess->ctx, sess->fd, sess->line);
        sess->line_len    = 0;
        sess->history_pos = -1;
        sess->esc_len     = 0;
        if (!sess->ctx->quit) {
            nv_core_cli_print_prompt(sess->fd);
        }
        break;
    }
}

static void nv_telnet_echo_char(nv_telnet_sess_t *sess, unsigned char c)
{
    char out[4];

    if (sess->state == NV_TELNET_ST_PASS) {
        out[0] = '*';
        out[1] = '\0';
    } else {
        out[0] = (char)c;
        out[1] = '\0';
    }
    nv_telnet_write_raw(sess->fd, out, 1);
}

static void nv_telnet_echo_backspace(nv_telnet_sess_t *sess)
{
    if (sess->state == NV_TELNET_ST_PASS) {
        nv_core_cli_write(sess->fd, "\b \b");
    } else {
        nv_core_cli_write(sess->fd, "\b \b");
    }
}

static void nv_telnet_tab_complete(nv_telnet_sess_t *sess)
{
    if (!sess || sess->state != NV_TELNET_ST_SHELL) {
        return;
    }

    if (sess->history_pos >= 0) {
        sess->history_pos = -1;
    }

    sess->line[sess->line_len] = '\0';
    nv_core_cli_tab_complete(sess->fd, sess->line, sizeof(sess->line), &sess->line_len);
    nv_telnet_redraw_line(sess);
}

/* 过滤 Telnet IAC 序列，将用户输入写入 line */
static void nv_telnet_feed(nv_telnet_sess_t *sess, const unsigned char *data, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++) {
        unsigned char c = data[i];

        if (sess->esc_len > 0 || c == 0x1b) {
            if (nv_telnet_escape_feed(sess, c)) {
                continue;
            }
        }

        if (c == 255) {
            /* IAC: 跳过 telnet 选项协商字节 */
            if (i + 1 < len) {
                unsigned char cmd = data[i + 1];
                if (cmd >= 251 && cmd <= 254 && i + 2 < len) {
                    i += 2;
                } else if (cmd == 255) {
                    i += 1;
                } else {
                    i += 1;
                }
            }
            continue;
        }

        /* Telnet 回车通常只发 CR(0x0D)，必须在此提交行 */
        if (c == '\r') {
            nv_telnet_process_line(sess);
            continue;
        }
        if (c == '\n') {
            if (sess->line_len > 0) {
                nv_telnet_process_line(sess);
            }
            continue;
        }
        if (c == 127 || c == 8) {
            if (sess->line_len > 0) {
                sess->line_len--;
                nv_telnet_echo_backspace(sess);
            }
            continue;
        }
        if (c == '\t') {
            if (sess->state == NV_TELNET_ST_SHELL) {
                nv_telnet_tab_complete(sess);
            }
            continue;
        }
        if (c < 32) {
            continue;
        }
        if (sess->line_len < sizeof(sess->line) - 1) {
            if (sess->state == NV_TELNET_ST_SHELL && sess->history_pos >= 0) {
                sess->history_pos = -1;
                sess->line_len    = 0;
                sess->line[0]     = '\0';
            }
            sess->line[sess->line_len++] = (char)c;
            nv_telnet_echo_char(sess, c);
        }
    }
}

static void nv_telnet_client_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_telnet_sess_t *sess = (nv_telnet_sess_t *)data;
    unsigned char     buf[512];
    ssize_t           n;

    (void)loop;
    (void)ev;

    if (!sess || sess->fd < 0) {
        return;
    }

    n = read(sess->fd, buf, sizeof(buf));
    if (n <= 0) {
        nv_log_info("telnet client disconnected fd=%d", sess->fd);
        nv_telnet_sess_remove(sess);
        return;
    }

    nv_telnet_feed(sess, buf, (size_t)n);

    if (sess->ctx && sess->ctx->quit) {
        nv_telnet_sess_remove(sess);
    }
}

static int nv_telnet_sess_start(nv_core_ctx_t *ctx, int cfd)
{
    nv_telnet_sess_t *sess;
    int               flags;

    sess = (nv_telnet_sess_t *)calloc(1, sizeof(*sess));
    if (!sess) {
        return NV_ERROR;
    }

    sess->active = 1;
    sess->fd     = cfd;
    sess->ctx    = ctx;
    sess->state         = NV_TELNET_ST_USER;
    sess->line_len      = 0;
    sess->history_count = 0;
    sess->history_pos   = -1;
    sess->esc_len       = 0;

    flags = fcntl(cfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
    }

    nv_telnet_write_raw(cfd, g_telnet_init, sizeof(g_telnet_init));

    memset(&sess->ev, 0, sizeof(sess->ev));
    sess->ev.fd      = cfd;
    sess->ev.type    = NV_EVENT_TYPE_IO;
    sess->ev.handler = nv_telnet_client_handler;
    sess->ev.data    = sess;

    if (nv_loop_add_event(&ctx->loop, &sess->ev, EPOLLIN) != 0) {
        nv_telnet_sess_free(sess);
        return NV_ERROR;
    }

    sess->next         = g_telnet_sessions;
    g_telnet_sessions  = sess;

    nv_telnet_send_login_user(sess);
    nv_log_info("telnet session from fd=%d", cfd);
    return NV_OK;
}

static void nv_telnet_accept_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_core_ctx_t  *ctx = (nv_core_ctx_t *)data;
    nv_event_ext_t *lev = (nv_event_ext_t *)ev;
    int             cfd;

    (void)loop;

    if (!ctx || !lev || lev->fd < 0) {
        return;
    }

    for (;;) {
        cfd = accept(lev->fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                break;
            }
            break;
        }
        if (nv_telnet_sess_start(ctx, cfd) != NV_OK) {
            close(cfd);
        }
    }
}

int nv_core_telnet_init(nv_core_ctx_t *ctx)
{
    struct sockaddr_in addr;
    int                sfd;
    int                on = 1;
    int                flags;

    if (!ctx || !ctx->opts.telnet_enable) {
        return NV_OK;
    }

    if (ctx->telnet_inited) {
        return NV_OK;
    }

    sfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sfd < 0) {
        nv_log_error("telnet socket failed: %s", strerror(errno));
        return NV_ERROR;
    }

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)ctx->opts.telnet_port);

    if (!ctx->opts.telnet_bind || !ctx->opts.telnet_bind[0] ||
        strcmp(ctx->opts.telnet_bind, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (inet_pton(AF_INET, ctx->opts.telnet_bind, &addr.sin_addr) != 1) {
        nv_log_error("telnet invalid bind address: %s", ctx->opts.telnet_bind);
        close(sfd);
        return NV_ERROR;
    }

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        nv_log_error("telnet bind %s:%d failed: %s",
                     ctx->opts.telnet_bind ? ctx->opts.telnet_bind : "0.0.0.0",
                     ctx->opts.telnet_port, strerror(errno));
        close(sfd);
        return NV_ERROR;
    }

    if (listen(sfd, NV_TELNET_BACKLOG) < 0) {
        close(sfd);
        return NV_ERROR;
    }

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
    }

    memset(&ctx->telnet_listen_ev, 0, sizeof(ctx->telnet_listen_ev));
    ctx->telnet_listen_ev.fd      = sfd;
    ctx->telnet_listen_ev.type    = NV_EVENT_TYPE_IO;
    ctx->telnet_listen_ev.handler = nv_telnet_accept_handler;
    ctx->telnet_listen_ev.data    = ctx;
    ctx->telnet_listen_fd         = sfd;

    if (nv_loop_add_event(&ctx->loop, &ctx->telnet_listen_ev, EPOLLIN) != 0) {
        close(sfd);
        ctx->telnet_listen_fd = -1;
        return NV_ERROR;
    }

    ctx->telnet_inited = 1;
    nv_log_info("telnet CLI listening on %s:%d",
                ctx->opts.telnet_bind ? ctx->opts.telnet_bind : "0.0.0.0",
                ctx->opts.telnet_port);
    return NV_OK;
}

void nv_core_telnet_cleanup(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    while (g_telnet_sessions) {
        nv_telnet_sess_remove(g_telnet_sessions);
    }

    if (ctx->telnet_inited && ctx->telnet_listen_fd >= 0) {
        nv_loop_del_event(&ctx->loop, &ctx->telnet_listen_ev);
        close(ctx->telnet_listen_fd);
        ctx->telnet_listen_fd = -1;
    }

    ctx->telnet_inited = 0;
}
