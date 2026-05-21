/************************************************
 * nv_core_args.c - 命令行与 INI 配置
 ***********************************************/

#include "nv_core_private.h"
#include "nv_version.h"

#include <nv_log.h>
#include <nv_ini.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void nv_core_set_defaults(nv_core_ctx_t *ctx)
{
    memset(&ctx->opts, 0, sizeof(ctx->opts));
    ctx->opts.config_file = NV_CORE_DEFAULT_CONFIG;
    ctx->opts.pid_file    = NV_CORE_DEFAULT_PID_FILE;
    ctx->opts.log_file    = NV_CORE_DEFAULT_LOG_FILE;
    ctx->opts.mq_name     = "/nv_core_mq";
    ctx->opts.daemon      = 0;
    ctx->opts.foreground  = 0;
    ctx->opts.use_syslog  = 0;
    ctx->opts.log_level   = NV_LOG_LEVEL_INFO;
    ctx->opts.worker_threads          = 1;
    ctx->opts.worker_connections      = 128;
    ctx->opts.heartbeat_interval_sec  = 60;
    ctx->opts.shutdown_timeout_sec    = 30;
    ctx->opts.systemd_notify          = NV_CORE_SYSTEMD_AUTO;
    ctx->opts.worker_processes        = 0;
    ctx->opts.worker_respawn          = 1;
    ctx->opts.max_open_files          = 0;
    ctx->opts.core_limit              = -1;
    ctx->opts.ctl_socket              = NV_CORE_DEFAULT_CTL_SOCKET;
    ctx->opts.pubsub_socket           = NV_CORE_DEFAULT_PUBSUB_SOCKET;
    ctx->opts.instance_lock           = NV_CORE_DEFAULT_LOCK_NAME;
    ctx->opts.telnet_enable           = 0;
    ctx->opts.telnet_port             = NV_CORE_DEFAULT_TELNET_PORT;
    ctx->opts.telnet_bind             = NV_CORE_DEFAULT_TELNET_BIND;
    ctx->opts.cli_username            = "admin";
    ctx->opts.cli_password            = "";
    ctx->opts.loadlib_dir              = NV_CORE_DEFAULT_LOADLIB_DIR;
    ctx->opts.loadlib_allow_absolute    = 0;
    ctx->opts.tombstone_file            = NV_CORE_DEFAULT_TOMBSTONE_FILE;
    ctx->opts.watchdog_enable           = 0;
    ctx->opts.watchdog_device            = "/dev/watchdog";
    ctx->opts.watchdog_cmd              = "";
    ctx->opts.log_rotate_max_mb          = 4;
    ctx->opts.log_rotate_keep            = 3;
    ctx->opts.metrics_publish            = 0;
    ctx->opts.metrics_topic              = NV_CORE_DEFAULT_METRICS_TOPIC;

    memset(&ctx->conf, 0, sizeof(ctx->conf));
    ctx->conf.daemon             = 0;
    ctx->conf.worker_processes   = ctx->opts.worker_threads;
    ctx->conf.worker_connections = ctx->opts.worker_connections;
    ctx->conf.pid_file           = (char *)ctx->opts.pid_file;
    ctx->conf.log_file           = (char *)ctx->opts.log_file;
    ctx->conf.log_level          = ctx->opts.log_level;

    ctx->phase       = NV_CORE_PHASE_NONE;
    ctx->pid_fd      = -1;
    ctx->thread_pool = NULL;
    ctx->msg_queue   = NULL;
    ctx->quit        = 0;
    ctx->reload      = 0;
    ctx->restart     = 0;
    ctx->started_at  = 0;
    ctx->shutdown_started_at = 0;
    ctx->health_inited = 0;
    ctx->instance_lock_fd = -1;
    ctx->ctl_listen_fd    = -1;
    ctx->ctl_inited       = 0;
    ctx->telnet_listen_fd = -1;
    ctx->telnet_inited    = 0;
    ctx->is_master        = 0;
    ctx->is_worker        = 0;
    ctx->worker_id        = 0;
    ctx->ini         = NULL;
    ctx->pid_file_dup = NULL;
    ctx->log_file_dup = NULL;
    ctx->mq_name_dup  = NULL;
    ctx->ctl_socket_dup = NULL;
    ctx->pubsub_socket_dup = NULL;
    ctx->instance_lock_dup = NULL;
    ctx->telnet_bind_dup = NULL;
    ctx->cli_username_dup = NULL;
    ctx->cli_password_dup = NULL;
    ctx->loadlib_dir_dup = NULL;
    ctx->tombstone_file_dup = NULL;
    ctx->watchdog_device_dup = NULL;
    ctx->watchdog_cmd_dup = NULL;
    ctx->metrics_topic_dup = NULL;
    ctx->loadlibs = NULL;
    ctx->loadlibs_loaded = 0;
    ctx->pubsubs = NULL;
    ctx->pubsub_inited = 0;
    ctx->pubsub_next_id = 0;
    ctx->pubsub_ipc_fd = -1;
    ctx->pubsub_ipc_running = 0;
    ctx->pubsub_ipc_thread_started = 0;
    ctx->pubsub_ipc_clients = NULL;
}

void nv_core_cfg_set_str(char **slot, const char *val)
{
    if (!slot || !val) {
        return;
    }
    free(*slot);
    *slot = strdup(val);
}

const char *nv_core_phase_name(nv_core_phase_t phase)
{
    switch (phase) {
    case NV_CORE_PHASE_STARTUP:    return "startup";
    case NV_CORE_PHASE_BUSINESS:   return "business";
    case NV_CORE_PHASE_LOOP:       return "event_loop";
    case NV_CORE_PHASE_SHUTDOWN:   return "shutdown";
    case NV_CORE_PHASE_EXCEPTION:  return "exception";
    default:                       return "none";
    }
}

void nv_core_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog ? prog : "nv");
    printf("  -c <file>   config file (default %s)\n", NV_CORE_DEFAULT_CONFIG);
    printf("  -p <file>   pid file (default %s)\n", NV_CORE_DEFAULT_PID_FILE);
    printf("  -l <file>   log file (default %s)\n", NV_CORE_DEFAULT_LOG_FILE);
    printf("  -d          run as daemon\n");
    printf("  -f          run in foreground (override daemon)\n");
    printf("  -s          also log to syslog\n");
    printf("  -h          show help\n");
    printf("  -v          show version\n");
    printf("\nSignals: SIGINT/SIGTERM quit, SIGHUP reload, SIGUSR1 restart\n");
    printf("Control: echo status | socat - UNIX:%s  (CLI cannot stop daemon)\n",
           NV_CORE_DEFAULT_CTL_SOCKET);
    printf("Telnet:  telnet %s %d  (default user admin)\n",
           NV_CORE_DEFAULT_TELNET_BIND, NV_CORE_DEFAULT_TELNET_PORT);
}

int nv_core_parse_args(nv_core_ctx_t *ctx, int argc, char **argv)
{
    int opt;

    if (!ctx) {
        return NV_ERROR;
    }

    nv_core_set_defaults(ctx);
    ctx->opts.prog_name = (argc > 0 && argv[0]) ? argv[0] : "nv";

    while ((opt = getopt(argc, argv, "c:p:l:dfshv")) != -1) {
        switch (opt) {
        case 'c': ctx->opts.config_file = optarg; break;
        case 'p': ctx->opts.pid_file    = optarg; break;
        case 'l': ctx->opts.log_file    = optarg; break;
        case 'd': ctx->opts.daemon      = 1; break;
        case 'f': ctx->opts.foreground = 1; break;
        case 's': ctx->opts.use_syslog = 1; break;
        case 'h': ctx->opts.help       = 1; break;
        case 'v': ctx->opts.version    = 1; break;
        default:
            nv_core_usage(ctx->opts.prog_name);
            return NV_ERROR;
        }
    }

    if (ctx->opts.help) {
        nv_core_usage(ctx->opts.prog_name);
        return NV_DECLINED;
    }
    if (ctx->opts.version) {
        printf("libnv %s\n", nv_version_string());
        printf("build time: %s\n", nv_build_time_string());
        return NV_DECLINED;
    }

    ctx->conf.pid_file = (char *)ctx->opts.pid_file;
    ctx->conf.log_file = (char *)ctx->opts.log_file;
    ctx->conf.log_level = ctx->opts.log_level;
    return NV_OK;
}

int nv_core_parse_log_level(const char *v)
{
    nv_log_level_e level;

    if (!v) {
        return NV_LOG_LEVEL_INFO;
    }
    if (nv_log_level_from_string(v, &level) == 0) {
        return (int)level;
    }
    return NV_LOG_LEVEL_INFO;
}

int nv_core_load_config(nv_core_ctx_t *ctx)
{
    nv_ini_t    *ini;
    const char  *str;

    if (!ctx || !ctx->opts.config_file) {
        return NV_ERROR;
    }

    if (ctx->ini) {
        nv_ini_free(ctx->ini);
        ctx->ini = NULL;
    }

    ini = nv_ini_load(ctx->opts.config_file);
    if (!ini) {
        nv_log_warning("config file not found: %s, use defaults", ctx->opts.config_file);
        return NV_OK;
    }
    ctx->ini = ini;

    /* [loadlib] 须在解析顶层 loadlib 指令之前加载 */
    str = nv_ini_get_string(ini, "loadlib", "plugin_dir", NULL);
    if (!str) {
        str = nv_ini_get_string(ini, "loadlib", "dir", NULL);
    }
    if (str) {
        nv_core_cfg_set_str(&ctx->loadlib_dir_dup, str);
        ctx->opts.loadlib_dir = ctx->loadlib_dir_dup;
    }
    if (nv_ini_has_key(ini, "loadlib", "allow_absolute")) {
        ctx->opts.loadlib_allow_absolute =
            nv_ini_get_bool(ini, "loadlib", "allow_absolute",
                            ctx->opts.loadlib_allow_absolute);
    }

    if (nv_core_loadlibs_parse_config(ctx) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_ini_has_key(ini, "main", "daemon")) {
        ctx->opts.daemon = nv_ini_get_bool(ini, "main", "daemon", ctx->opts.daemon);
    }

    str = nv_ini_get_string(ini, "main", "pid_file", NULL);
    if (!str) {
        str = nv_ini_get_string(ini, "main", "pid", NULL);
    }
    if (str) {
        nv_core_cfg_set_str(&ctx->pid_file_dup, str);
        ctx->opts.pid_file = ctx->pid_file_dup;
        ctx->conf.pid_file = ctx->pid_file_dup;
    }

    str = nv_ini_get_string(ini, "log", "log_file", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->log_file_dup, str);
        ctx->opts.log_file = ctx->log_file_dup;
        ctx->conf.log_file = ctx->log_file_dup;
    }

    str = nv_ini_get_string(ini, "log", "log_level", NULL);
    if (str) {
        ctx->opts.log_level = nv_core_parse_log_level(str);
        ctx->conf.log_level = ctx->opts.log_level;
    }

    if (nv_ini_has_key(ini, "log", "syslog")) {
        ctx->opts.use_syslog = nv_ini_get_bool(ini, "log", "syslog", ctx->opts.use_syslog);
    }

    if (nv_ini_has_key(ini, "log", "console")) {
        nv_log_set_console(nv_ini_get_bool(ini, "log", "console", nv_log_get_console()));
    }

    if (nv_ini_has_key(ini, "log", "queue_size")) {
        nv_log_set_queue_size(
            (size_t)nv_ini_get_int(ini, "log", "queue_size",
                                   (int)nv_log_get_queue_size()));
    }

    str = nv_ini_get_string(ini, "log", "overflow", NULL);
    if (str) {
        nv_log_overflow_e pol;
        if (nv_log_overflow_from_string(str, &pol) == 0) {
            nv_log_set_overflow_policy(pol);
        }
    }

    if (nv_ini_has_key(ini, "worker", "threads")) {
        ctx->opts.worker_threads =
            nv_ini_get_int(ini, "worker", "threads", ctx->opts.worker_threads);
    } else if (nv_ini_has_key(ini, "worker", "worker_threads")) {
        ctx->opts.worker_threads =
            nv_ini_get_int(ini, "worker", "worker_threads", ctx->opts.worker_threads);
    }

    if (nv_ini_has_key(ini, "worker", "connections")) {
        ctx->opts.worker_connections =
            nv_ini_get_int(ini, "worker", "connections", ctx->opts.worker_connections);
    } else if (nv_ini_has_key(ini, "worker", "worker_connections")) {
        ctx->opts.worker_connections =
            nv_ini_get_int(ini, "worker", "worker_connections", ctx->opts.worker_connections);
    }

    if (nv_ini_has_key(ini, "worker", "processes")) {
        ctx->opts.worker_processes =
            nv_ini_get_int(ini, "worker", "processes", ctx->opts.worker_processes);
    }
    if (nv_ini_has_key(ini, "worker", "respawn")) {
        ctx->opts.worker_respawn =
            nv_ini_get_bool(ini, "worker", "respawn", ctx->opts.worker_respawn);
    }

    str = nv_ini_get_string(ini, "ipc", "mq_name", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->mq_name_dup, str);
        ctx->opts.mq_name = ctx->mq_name_dup;
    }

    if (nv_ini_has_key(ini, "daemon", "heartbeat_interval")) {
        ctx->opts.heartbeat_interval_sec =
            nv_ini_get_int(ini, "daemon", "heartbeat_interval",
                           ctx->opts.heartbeat_interval_sec);
    }
    if (nv_ini_has_key(ini, "daemon", "shutdown_timeout")) {
        ctx->opts.shutdown_timeout_sec =
            nv_ini_get_int(ini, "daemon", "shutdown_timeout",
                           ctx->opts.shutdown_timeout_sec);
    }
    if (nv_ini_has_key(ini, "daemon", "systemd_notify")) {
        ctx->opts.systemd_notify =
            nv_ini_get_bool(ini, "daemon", "systemd_notify",
                            ctx->opts.systemd_notify > 0) ? 1 : 0;
    }

    if (nv_ini_has_key(ini, "daemon", "max_open_files")) {
        ctx->opts.max_open_files =
            nv_ini_get_int(ini, "daemon", "max_open_files", ctx->opts.max_open_files);
    }
    if (nv_ini_has_key(ini, "daemon", "core_limit")) {
        ctx->opts.core_limit =
            (long long)nv_ini_get_int64(ini, "daemon", "core_limit",
                                        (int64_t)ctx->opts.core_limit);
    }

    str = nv_ini_get_string(ini, "daemon", "ctl_socket", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->ctl_socket_dup, str);
        ctx->opts.ctl_socket = ctx->ctl_socket_dup;
    }

    str = nv_ini_get_string(ini, "ipc", "pubsub_socket", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->pubsub_socket_dup, str);
        ctx->opts.pubsub_socket = ctx->pubsub_socket_dup;
    }

    str = nv_ini_get_string(ini, "daemon", "instance_lock", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->instance_lock_dup, str);
        ctx->opts.instance_lock = ctx->instance_lock_dup;
    }

    if (nv_ini_has_key(ini, "cli", "telnet_enable")) {
        ctx->opts.telnet_enable =
            nv_ini_get_bool(ini, "cli", "telnet_enable", ctx->opts.telnet_enable);
    }
    if (nv_ini_has_key(ini, "cli", "telnet_port")) {
        ctx->opts.telnet_port =
            nv_ini_get_int(ini, "cli", "telnet_port", ctx->opts.telnet_port);
    }
    str = nv_ini_get_string(ini, "cli", "telnet_bind", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->telnet_bind_dup, str);
        ctx->opts.telnet_bind = ctx->telnet_bind_dup;
    }
    str = nv_ini_get_string(ini, "cli", "username", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->cli_username_dup, str);
        ctx->opts.cli_username = ctx->cli_username_dup;
    }
    str = nv_ini_get_string(ini, "cli", "password", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->cli_password_dup, str);
        ctx->opts.cli_password = ctx->cli_password_dup;
    }

    str = nv_ini_get_string(ini, "daemon", "tombstone_file", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->tombstone_file_dup, str);
        ctx->opts.tombstone_file = ctx->tombstone_file_dup;
    }

    if (nv_ini_has_key(ini, "watchdog", "enable")) {
        ctx->opts.watchdog_enable =
            nv_ini_get_bool(ini, "watchdog", "enable", ctx->opts.watchdog_enable);
    }
    str = nv_ini_get_string(ini, "watchdog", "device", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->watchdog_device_dup, str);
        ctx->opts.watchdog_device = ctx->watchdog_device_dup;
    }
    str = nv_ini_get_string(ini, "watchdog", "cmd", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->watchdog_cmd_dup, str);
        ctx->opts.watchdog_cmd = ctx->watchdog_cmd_dup;
    }

    if (nv_ini_has_key(ini, "log", "rotate_max_mb")) {
        ctx->opts.log_rotate_max_mb =
            nv_ini_get_int(ini, "log", "rotate_max_mb", ctx->opts.log_rotate_max_mb);
    }
    if (nv_ini_has_key(ini, "log", "rotate_keep")) {
        ctx->opts.log_rotate_keep =
            nv_ini_get_int(ini, "log", "rotate_keep", ctx->opts.log_rotate_keep);
    }
    if (ctx->opts.log_rotate_max_mb > 0 && ctx->opts.log_rotate_keep < 1) {
        ctx->opts.log_rotate_keep = 1;
    }
    if (ctx->opts.log_rotate_max_mb > 0) {
        nv_log_set_rotate((size_t)ctx->opts.log_rotate_max_mb * 1024U * 1024U,
                          ctx->opts.log_rotate_keep);
    }

    if (nv_ini_has_key(ini, "metrics", "publish")) {
        ctx->opts.metrics_publish =
            nv_ini_get_bool(ini, "metrics", "publish", ctx->opts.metrics_publish);
    }
    str = nv_ini_get_string(ini, "metrics", "topic", NULL);
    if (str) {
        nv_core_cfg_set_str(&ctx->metrics_topic_dup, str);
        ctx->opts.metrics_topic = ctx->metrics_topic_dup;
    }

    ctx->conf.daemon             = ctx->opts.daemon;
    ctx->conf.worker_processes   = ctx->opts.worker_processes > 0
                                   ? ctx->opts.worker_processes
                                   : ctx->opts.worker_threads;
    ctx->conf.worker_connections = ctx->opts.worker_connections;
    return NV_OK;
}
