/************************************************
 * @文件名: nv_core.c
 * @功能: libnv 主进程框架实现
 ***********************************************/

#include "nv_core.h"
#include "nv_core_cli.h"
#include "nv_core_health.h"
#include "nv_core_config.h"
#include "nv_core_ctl.h"
#include "nv_core_telnet.h"
#include "nv_core_lock.h"
#include "nv_core_worker.h"
#include "nv_core_modules.h"
#include "nv_event.h"
#include "nv_version.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <syslog.h>
#include <signal.h>

static nv_core_ctx_t        *g_core_ctx    = NULL;
static const nv_core_hooks_t *g_core_hooks  = NULL;
static nv_event_ext_t        g_idle_ev;
char                       **g_saved_argv   = NULL;
static int                   g_saved_argc   = 0;

static void nv_core_set_defaults(nv_core_ctx_t *ctx)
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
    ctx->opts.worker_threads          = 4;
    ctx->opts.worker_connections      = 1024;
    ctx->opts.heartbeat_interval_sec  = 60;
    ctx->opts.shutdown_timeout_sec    = 30;
    ctx->opts.systemd_notify          = NV_CORE_SYSTEMD_AUTO;
    ctx->opts.worker_processes        = 0;
    ctx->opts.worker_respawn          = 1;
    ctx->opts.max_open_files          = 0;
    ctx->opts.core_limit              = -1;
    ctx->opts.ctl_socket              = NV_CORE_DEFAULT_CTL_SOCKET;
    ctx->opts.instance_lock           = NV_CORE_DEFAULT_LOCK_NAME;
    ctx->opts.telnet_enable           = 1;
    ctx->opts.telnet_port             = NV_CORE_DEFAULT_TELNET_PORT;
    ctx->opts.telnet_bind             = NV_CORE_DEFAULT_TELNET_BIND;
    ctx->opts.cli_username            = "admin";
    ctx->opts.cli_password            = "nvadmin";

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
    ctx->instance_lock_dup = NULL;
    ctx->telnet_bind_dup = NULL;
    ctx->cli_username_dup = NULL;
    ctx->cli_password_dup = NULL;
}

static void nv_core_cfg_set_str(char **slot, const char *val)
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

static int nv_core_parse_log_level(const char *v)
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

    ctx->conf.daemon             = ctx->opts.daemon;
    ctx->conf.worker_processes   = ctx->opts.worker_processes > 0
                                   ? ctx->opts.worker_processes
                                   : ctx->opts.worker_threads;
    ctx->conf.worker_connections = ctx->opts.worker_connections;
    return NV_OK;
}

static int nv_core_log_init(nv_core_ctx_t *ctx)
{
    if (nv_log_init_file(ctx->opts.log_file) != 0) {
        nv_log_warning("open log file failed: %s", ctx->opts.log_file);
        if (nv_log_init() != 0) {
            return NV_ERROR;
        }
    }
    nv_log_set_level((nv_log_level_e)ctx->opts.log_level);
    if (ctx->opts.use_syslog) {
        nv_log_init_syslog(ctx->opts.prog_name ? ctx->opts.prog_name : "nv");
    }
    return NV_OK;
}

static int nv_core_pidfile_create(nv_core_ctx_t *ctx)
{
    char buf[32];
    int fd;
    struct flock lock;

    if (!ctx->opts.pid_file) {
        return NV_OK;
    }

    fd = open(ctx->opts.pid_file, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        nv_log_error("pid file open failed: %s (%s)", ctx->opts.pid_file, strerror(errno));
        return NV_ERROR;
    }

    memset(&lock, 0, sizeof(lock));
    lock.l_type   = F_WRLCK;
    lock.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &lock) < 0) {
        nv_log_error("another instance is running (pid file: %s)", ctx->opts.pid_file);
        close(fd);
        return NV_BUSY;
    }

    if (ftruncate(fd, 0) == 0) {
        ssize_t n;
        size_t  len;

        snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
        len = strlen(buf);
        n   = write(fd, buf, len);
        if (n < 0 || (size_t)n != len) {
            nv_log_warning("pid file write failed: %s (%s)",
                           ctx->opts.pid_file, strerror(errno));
        }
    }

    ctx->pid_fd = fd;
    return NV_OK;
}

static void nv_core_pidfile_remove(nv_core_ctx_t *ctx)
{
    if (ctx->pid_fd >= 0) {
        close(ctx->pid_fd);
        ctx->pid_fd = -1;
    }
    if (ctx->opts.pid_file) {
        unlink(ctx->opts.pid_file);
    }
}

static void nv_core_on_signal(int signum)
{
    if (!g_core_ctx) {
        return;
    }

    switch (signum) {
    case SIGINT:
    case SIGTERM:
        g_core_ctx->quit = 1;
        if (!g_core_ctx->shutdown_started_at) {
            g_core_ctx->shutdown_started_at = time(NULL);
        }
        nv_loop_stop(&g_core_ctx->loop);
        break;
    case SIGHUP:
        g_core_ctx->reload = 1;
        break;
    case SIGUSR1:
        g_core_ctx->restart = 1;
        g_core_ctx->quit = 1;
        if (!g_core_ctx->shutdown_started_at) {
            g_core_ctx->shutdown_started_at = time(NULL);
        }
        nv_loop_stop(&g_core_ctx->loop);
        break;
    default:
        break;
    }
}

static int nv_core_signals_register_loop(nv_core_ctx_t *ctx)
{
    if (nv_signal_init() != 0) {
        return NV_ERROR;
    }
    if (nv_signal_register(SIGINT,  nv_core_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGTERM, nv_core_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGHUP,  nv_core_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGUSR1, nv_core_on_signal) != 0) return NV_ERROR;
    if (ctx->opts.worker_processes > 1) {
        nv_signal_register(SIGCHLD, nv_core_on_signal);
    }
    signal(SIGPIPE, SIG_IGN);

    if (nv_loop_attach_signalfd(&ctx->loop, nv_signal_fd()) != 0) {
        nv_log_error("attach signalfd to loop failed");
        return NV_ERROR;
    }
    return NV_OK;
}

static void nv_core_apply_rlimits(nv_core_ctx_t *ctx)
{
    struct rlimit rl;

    if (!ctx) {
        return;
    }

    if (ctx->opts.max_open_files > 0) {
        rl.rlim_cur = (rlim_t)ctx->opts.max_open_files;
        rl.rlim_max = (rlim_t)ctx->opts.max_open_files;
        if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
            nv_log_info("RLIMIT_NOFILE set to %d", ctx->opts.max_open_files);
        } else {
            nv_log_warning("set RLIMIT_NOFILE failed: %s", strerror(errno));
        }
    }

    if (ctx->opts.core_limit == 0) {
        rl.rlim_cur = 0;
        rl.rlim_max = 0;
        setrlimit(RLIMIT_CORE, &rl);
        nv_log_info("core dumps disabled (RLIMIT_CORE=0)");
    } else if (ctx->opts.core_limit > 0) {
        rl.rlim_cur = (rlim_t)ctx->opts.core_limit;
        rl.rlim_max = (rlim_t)ctx->opts.core_limit;
        setrlimit(RLIMIT_CORE, &rl);
        nv_log_info("RLIMIT_CORE set to %lld", (long long)ctx->opts.core_limit);
    }
}

int nv_core_startup_init(nv_core_ctx_t *ctx)
{
    int run_daemon;

    if (!ctx) {
        return NV_ERROR;
    }

    g_core_ctx = ctx;
    ctx->phase = NV_CORE_PHASE_STARTUP;

    nv_core_exception_init();

    /* 命令行优先：前台模式先打开终端日志 */
    nv_log_set_console(ctx->opts.foreground || !ctx->opts.daemon);

    if (nv_core_load_config(ctx) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_core_config_validate(ctx) != NV_OK) {
        return NV_ERROR;
    }

    nv_core_apply_rlimits(ctx);

    run_daemon = ctx->opts.daemon && !ctx->opts.foreground;
    if (run_daemon) {
        nv_log_set_console(0);
    } else if (!ctx->ini || !nv_ini_has_key(ctx->ini, "log", "console")) {
        nv_log_set_console(1);
    }

    if (run_daemon) {
        if (nv_create_daemon() != 0) {
            nv_log_error("daemonize failed");
            return NV_ERROR;
        }
        ctx->conf.daemon = 1;
    }

    if (nv_core_log_init(ctx) != NV_OK) {
        return NV_ERROR;
    }

    nv_log_set_module("MAIN");
    nv_log_notice("libnv %s, build time %s",
                  nv_version_string(), nv_build_time_string());
    nv_log_info("系统初始化完成 [%s]", nv_core_phase_name(ctx->phase));

    if (nv_core_instance_lock_acquire(ctx->opts.instance_lock,
                                      &ctx->instance_lock_fd) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_core_pidfile_create(ctx) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_base_init(&ctx->conf) != NV_OK) {
        nv_log_error("core init failed");
        return NV_ERROR;
    }

    return NV_OK;
}

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

    ctx->msg_queue = nv_init_message_queue(ctx->opts.mq_name, 64, MAX_MSG_SIZE, 0);
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

void nv_core_exception_init(void)
{
    signal(SIGSEGV, nv_core_exception_handler);
    signal(SIGABRT, nv_core_exception_handler);
    signal(SIGFPE,  nv_core_exception_handler);
    signal(SIGILL, nv_core_exception_handler);
}

void nv_core_exception_handler(int signum)
{
    if (g_core_ctx) {
        g_core_ctx->phase = NV_CORE_PHASE_EXCEPTION;
    }

    nv_log_fatal("fatal signal caught: %d (%s)", signum, strsignal(signum));
    syslog(LOG_CRIT, "nv main fatal signal: %d", signum);

    if (g_core_ctx) {
        g_core_ctx->quit = 1;
        nv_loop_stop(&g_core_ctx->loop);
        nv_core_business_cleanup(g_core_ctx);
        nv_base_cleanup();
        nv_core_pidfile_remove(g_core_ctx);
        nv_log_close();
        if (g_core_ctx->opts.use_syslog) {
            closelog();
        }
        g_core_ctx = NULL;
    }

    signal(signum, SIG_DFL);
    raise(signum);
}

static void nv_core_check_shutdown_timeout(nv_core_ctx_t *ctx)
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

static void nv_core_idle_handler(nv_loop_t *loop, void *ev, void *data)
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

    if (g_core_hooks && g_core_hooks->on_idle) {
        g_core_hooks->on_idle(ctx);
    }
}

int nv_core_run_loop(nv_core_ctx_t *ctx, const nv_core_hooks_t *hooks)
{
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

    return nv_loop_run(&ctx->loop);
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

static void nv_core_free_saved_argv(void)
{
    int i;

    for (i = 0; i < g_saved_argc; i++) {
        free(g_saved_argv[i]);
    }
    free(g_saved_argv);
    g_saved_argv = NULL;
    g_saved_argc = 0;
}

static void nv_core_save_argv(int argc, char **argv)
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
