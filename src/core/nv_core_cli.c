#include "nv_core_cli.h"
#include "nv_core_config.h"
#include "nv_core_health.h"
#include "nv_version.h"

#include <nv_log.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int nv_core_cli_write(int fd, const char *fmt, ...)
{
    char    buf[NV_CORE_CLI_OUT_MAX];
    va_list ap;
    size_t  len;
    ssize_t n;

    if (fd < 0 || !fmt) {
        return NV_ERROR;
    }

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    len = strlen(buf);
    n   = write(fd, buf, len);
    if (n < 0 || (size_t)n != len) {
        return NV_ERROR;
    }
    return NV_OK;
}

void nv_core_cli_print_banner(int fd)
{
    nv_core_cli_write(fd,
        "\r\n"
        "  libnv management console\r\n"
        "  libnv %s  build %s\r\n"
        "\r\n",
        nv_version_string(), nv_build_time_string());
}

void nv_core_cli_print_prompt(int fd)
{
    nv_core_cli_write(fd, "nv> ");
}

static int nv_cli_cmd_help(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)ctx;
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd,
        "commands:\r\n"
        "  help              show this help\r\n"
        "  status            process and loop status\r\n"
        "  uptime            running time\r\n"
        "  version           libnv version\r\n"
        "  workers           worker/thread settings\r\n"
        "  config            key configuration\r\n"
        "  reload            reload config file\r\n"
        "  quit|exit         graceful shutdown\r\n"
        "  ping              connectivity test\r\n"
        "\r\n"
        "  Up/Down           command history (telnet)\r\n"
        "  Tab               command name completion\r\n");
    return NV_OK;
}

static int nv_cli_cmd_status(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    nv_core_runtime_stats_t st;
    nv_loop_stats_t         ls;

    (void)argc;
    (void)argv;

    nv_core_get_runtime_stats(ctx, &st);
    nv_loop_get_stats(&ctx->loop, &ls);

    nv_core_cli_write(fd,
        "phase:       %s\r\n"
        "pid:         %d\r\n"
        "worker_id:   %d\r\n"
        "is_master:   %d\r\n"
        "is_worker:   %d\r\n"
        "uptime:      %lu sec\r\n"
        "quitting:    %d\r\n"
        "reloading:   %d\r\n"
        "loop_events: %lu\r\n"
        "loop_timers: %lu\r\n"
        "loop_idle:   %lu\r\n"
        "log_level:   %d\r\n"
        "daemon:      %d\r\n",
        nv_core_phase_name(ctx->phase),
        (int)getpid(),
        ctx->worker_id,
        ctx->is_master,
        ctx->is_worker,
        st.uptime_sec,
        st.quitting,
        st.reloading,
        st.loop_events,
        st.loop_timers,
        st.loop_idle,
        ctx->opts.log_level,
        ctx->opts.daemon);
    return NV_OK;
}

static int nv_cli_cmd_uptime(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    nv_core_runtime_stats_t st;

    (void)argc;
    (void)argv;

    nv_core_get_runtime_stats(ctx, &st);
    nv_core_cli_write(fd, "uptime: %lu seconds\r\n", st.uptime_sec);
    return NV_OK;
}

static int nv_cli_cmd_version(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)ctx;
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd, "libnv %s\r\nbuild %s\r\n",
                      nv_version_string(), nv_build_time_string());
    return NV_OK;
}

static int nv_cli_cmd_workers(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd,
        "worker_processes:  %d\r\n"
        "worker_respawn:    %d\r\n"
        "worker_threads:    %d\r\n"
        "worker_connections:%d\r\n",
        ctx->opts.worker_processes,
        ctx->opts.worker_respawn,
        ctx->opts.worker_threads,
        ctx->opts.worker_connections);
    return NV_OK;
}

static int nv_cli_cmd_config(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd,
        "config_file:   %s\r\n"
        "pid_file:      %s\r\n"
        "log_file:      %s\r\n"
        "ctl_socket:    %s\r\n"
        "telnet:        %s:%d (%s)\r\n"
        "heartbeat:     %d sec\r\n"
        "shutdown_to:   %d sec\r\n",
        ctx->opts.config_file ? ctx->opts.config_file : "",
        ctx->opts.pid_file ? ctx->opts.pid_file : "",
        ctx->opts.log_file ? ctx->opts.log_file : "",
        ctx->opts.ctl_socket ? ctx->opts.ctl_socket : "",
        ctx->opts.telnet_bind ? ctx->opts.telnet_bind : "",
        ctx->opts.telnet_port,
        ctx->opts.telnet_enable ? "on" : "off",
        ctx->opts.heartbeat_interval_sec,
        ctx->opts.shutdown_timeout_sec);
    return NV_OK;
}

static int nv_cli_cmd_reload(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (nv_core_reload_config(ctx) == NV_OK) {
        nv_core_cli_write(fd, "OK config reloaded\r\n");
        return NV_OK;
    }
    nv_core_cli_write(fd, "ERR reload failed (rolled back)\r\n");
    return NV_ERROR;
}

static int nv_cli_cmd_quit(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd, "OK shutting down...\r\n");
    nv_core_request_quit(ctx);
    return NV_OK;
}

static int nv_cli_cmd_ping(nv_core_ctx_t *ctx, int fd, int argc, char **argv)
{
    (void)ctx;
    (void)argc;
    (void)argv;
    nv_core_cli_write(fd, "pong\r\n");
    return NV_OK;
}

typedef struct {
    const char *name;
    int (*handler)(nv_core_ctx_t *ctx, int fd, int argc, char **argv);
} nv_cli_cmd_t;

static const nv_cli_cmd_t g_cli_cmds[] = {
    { "help",    nv_cli_cmd_help },
    { "?",       nv_cli_cmd_help },
    { "status",  nv_cli_cmd_status },
    { "stat",    nv_cli_cmd_status },
    { "uptime",  nv_cli_cmd_uptime },
    { "version", nv_cli_cmd_version },
    { "ver",     nv_cli_cmd_version },
    { "workers", nv_cli_cmd_workers },
    { "config",  nv_cli_cmd_config },
    { "reload",  nv_cli_cmd_reload },
    { "quit",    nv_cli_cmd_quit },
    { "exit",    nv_cli_cmd_quit },
    { "shutdown",nv_cli_cmd_quit },
    { "ping",    nv_cli_cmd_ping },
    { NULL,      NULL },
};

static int nv_cli_split_args(char *line, char **argv, int max_argv)
{
    int argc = 0;
    char *p = line;

    while (*p && argc < max_argv - 1) {
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (!*p) {
            break;
        }
        argv[argc++] = p;
        while (*p && !isspace((unsigned char)*p)) {
            p++;
        }
        if (*p) {
            *p++ = '\0';
        }
    }
    argv[argc] = NULL;
    return argc;
}

int nv_core_cli_execute_line(nv_core_ctx_t *ctx, int fd, const char *line)
{
    char  buf[NV_CORE_CLI_LINE_MAX];
    char *argv[16];
    int   argc;
    int   i;

    if (!ctx || fd < 0 || !line) {
        return NV_ERROR;
    }

    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* trim trailing CR/LF */
    {
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\r' || buf[n - 1] == '\n')) {
            buf[--n] = '\0';
        }
    }

    if (buf[0] == '\0') {
        return NV_OK;
    }

    argc = nv_cli_split_args(buf, argv, 16);
    if (argc <= 0) {
        return NV_OK;
    }

    for (i = 0; g_cli_cmds[i].name; i++) {
        if (strcasecmp(argv[0], g_cli_cmds[i].name) == 0) {
            return g_cli_cmds[i].handler(ctx, fd, argc, argv);
        }
    }

    nv_core_cli_write(fd, "unknown command: %s (try help)\r\n", argv[0]);
    return NV_ERROR;
}

static size_t nv_cli_first_word_len(const char *line, size_t line_len)
{
    size_t i = 0;

    while (i < line_len && line[i] != ' ' && line[i] != '\t') {
        i++;
    }
    return i;
}

static size_t nv_cli_str_common_prefix(const char *a, const char *b)
{
    size_t i = 0;

    while (a[i] && b[i] &&
           tolower((unsigned char)a[i]) == tolower((unsigned char)b[i])) {
        i++;
    }
    return i;
}

static size_t nv_cli_matches_lcp(const char **matches, int count)
{
    size_t len;
    int    j;

    if (count <= 0) {
        return 0;
    }
    len = strlen(matches[0]);
    for (j = 1; j < count; j++) {
        size_t n = nv_cli_str_common_prefix(matches[0], matches[j]);
        if (n < len) {
            len = n;
        }
    }
    return len;
}

static int nv_cli_match_exists(const char **matches, int count, const char *name)
{
    int i;

    for (i = 0; i < count; i++) {
        if (strcasecmp(matches[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void nv_cli_build_completed_line(char *out, size_t out_size,
                                        const char *cmd, const char *tail)
{
    if (!tail || !tail[0]) {
        snprintf(out, out_size, "%s ", cmd);
    } else if (tail[0] == ' ' || tail[0] == '\t') {
        snprintf(out, out_size, "%s%s", cmd, tail);
    } else {
        snprintf(out, out_size, "%s %s", cmd, tail);
    }
}

int nv_core_cli_tab_complete(int fd, char *line, size_t line_max, size_t *line_len)
{
    const char *matches[32];
    char        tail[NV_CORE_CLI_LINE_MAX];
    char        new_line[NV_CORE_CLI_LINE_MAX];
    char        lcp_buf[NV_CORE_CLI_LINE_MAX];
    size_t      word_len;
    size_t      lcp_len;
    int         i;
    int         nmatch = 0;

    if (!line || !line_len || line_max < 2 || fd < 0) {
        return NV_ERROR;
    }

    line[*line_len] = '\0';
    word_len = nv_cli_first_word_len(line, *line_len);

    for (i = 0; g_cli_cmds[i].name && nmatch < 32; i++) {
        if (word_len == 0 ||
            strncasecmp(g_cli_cmds[i].name, line, word_len) == 0) {
            if (!nv_cli_match_exists(matches, nmatch, g_cli_cmds[i].name)) {
                matches[nmatch++] = g_cli_cmds[i].name;
            }
        }
    }

    if (nmatch == 0) {
        nv_core_cli_write(fd, "\a");
        return NV_OK;
    }

    tail[0] = '\0';
    if (*line_len > word_len) {
        strncpy(tail, line + word_len, sizeof(tail) - 1);
        tail[sizeof(tail) - 1] = '\0';
    }

    if (nmatch == 1) {
        nv_cli_build_completed_line(new_line, sizeof(new_line), matches[0], tail);
        strncpy(line, new_line, line_max - 1);
        line[line_max - 1] = '\0';
        *line_len = strlen(line);
        return NV_OK;
    }

    lcp_len = nv_cli_matches_lcp(matches, nmatch);
    if (lcp_len > word_len) {
        strncpy(lcp_buf, matches[0], lcp_len);
        lcp_buf[lcp_len] = '\0';
        if (!tail[0]) {
            snprintf(new_line, sizeof(new_line), "%s", lcp_buf);
        } else {
            snprintf(new_line, sizeof(new_line), "%s%s", lcp_buf, tail);
        }
        strncpy(line, new_line, line_max - 1);
        line[line_max - 1] = '\0';
        *line_len = strlen(line);
    }

    nv_core_cli_write(fd, "\r\n");
    for (i = 0; i < nmatch; i++) {
        nv_core_cli_write(fd, "%s  ", matches[i]);
    }
    nv_core_cli_write(fd, "\r\n");

    return NV_OK;
}
