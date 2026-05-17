/************************************************
 * @文件名: nv_main.c
 * @功能: libnv 主进程框架实现
 ***********************************************/

#include "nv_main.h"
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

static nv_main_ctx_t        *g_main_ctx   = NULL;
static const nv_main_hooks_t *g_main_hooks = NULL;
static nv_event_ext_t        g_idle_ev;

static void nv_main_set_defaults(nv_main_ctx_t *ctx)
{
    memset(&ctx->opts, 0, sizeof(ctx->opts));
    ctx->opts.config_file = NV_MAIN_DEFAULT_CONFIG;
    ctx->opts.pid_file    = NV_MAIN_DEFAULT_PID_FILE;
    ctx->opts.log_file    = NV_MAIN_DEFAULT_LOG_FILE;
    ctx->opts.mq_name     = "/nv_main_mq";
    ctx->opts.daemon      = 0;
    ctx->opts.foreground  = 0;
    ctx->opts.use_syslog  = 0;
    ctx->opts.log_level   = NV_LOG_LEVEL_INFO;
    ctx->opts.worker_threads     = 4;
    ctx->opts.worker_connections = 1024;

    memset(&ctx->conf, 0, sizeof(ctx->conf));
    ctx->conf.daemon             = 0;
    ctx->conf.worker_processes   = ctx->opts.worker_threads;
    ctx->conf.worker_connections = ctx->opts.worker_connections;
    ctx->conf.pid_file           = (char *)ctx->opts.pid_file;
    ctx->conf.log_file           = (char *)ctx->opts.log_file;
    ctx->conf.log_level          = ctx->opts.log_level;

    ctx->phase       = NV_MAIN_PHASE_NONE;
    ctx->pid_fd      = -1;
    ctx->thread_pool = NULL;
    ctx->msg_queue   = NULL;
    ctx->quit        = 0;
    ctx->reload      = 0;
    ctx->restart     = 0;
    ctx->ini         = NULL;
    ctx->pid_file_dup = NULL;
    ctx->log_file_dup = NULL;
    ctx->mq_name_dup  = NULL;
}

static void nv_main_cfg_set_str(char **slot, const char *val)
{
    if (!slot || !val) {
        return;
    }
    free(*slot);
    *slot = strdup(val);
}

const char *nv_main_phase_name(nv_main_phase_t phase)
{
    switch (phase) {
    case NV_MAIN_PHASE_STARTUP:    return "startup";
    case NV_MAIN_PHASE_BUSINESS:   return "business";
    case NV_MAIN_PHASE_LOOP:       return "event_loop";
    case NV_MAIN_PHASE_SHUTDOWN:   return "shutdown";
    case NV_MAIN_PHASE_EXCEPTION:  return "exception";
    default:                       return "none";
    }
}

void nv_main_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog ? prog : "nv");
    printf("  -c <file>   config file (default %s)\n", NV_MAIN_DEFAULT_CONFIG);
    printf("  -p <file>   pid file (default %s)\n", NV_MAIN_DEFAULT_PID_FILE);
    printf("  -l <file>   log file (default %s)\n", NV_MAIN_DEFAULT_LOG_FILE);
    printf("  -d          run as daemon\n");
    printf("  -f          run in foreground (override daemon)\n");
    printf("  -s          also log to syslog\n");
    printf("  -h          show help\n");
    printf("  -v          show version\n");
    printf("\nSignals: SIGINT/SIGTERM quit, SIGHUP reload, SIGUSR1 restart\n");
}

int nv_main_parse_args(nv_main_ctx_t *ctx, int argc, char **argv)
{
    int opt;

    if (!ctx) {
        return NV_ERROR;
    }

    nv_main_set_defaults(ctx);
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
            nv_main_usage(ctx->opts.prog_name);
            return NV_ERROR;
        }
    }

    if (ctx->opts.help) {
        nv_main_usage(ctx->opts.prog_name);
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

static int nv_main_parse_log_level(const char *v)
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

int nv_main_load_config(nv_main_ctx_t *ctx)
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
        nv_main_cfg_set_str(&ctx->pid_file_dup, str);
        ctx->opts.pid_file = ctx->pid_file_dup;
        ctx->conf.pid_file = ctx->pid_file_dup;
    }

    str = nv_ini_get_string(ini, "log", "log_file", NULL);
    if (str) {
        nv_main_cfg_set_str(&ctx->log_file_dup, str);
        ctx->opts.log_file = ctx->log_file_dup;
        ctx->conf.log_file = ctx->log_file_dup;
    }

    str = nv_ini_get_string(ini, "log", "log_level", NULL);
    if (str) {
        ctx->opts.log_level = nv_main_parse_log_level(str);
        ctx->conf.log_level = ctx->opts.log_level;
    }

    if (nv_ini_has_key(ini, "log", "syslog")) {
        ctx->opts.use_syslog = nv_ini_get_bool(ini, "log", "syslog", ctx->opts.use_syslog);
    }

    if (nv_ini_has_key(ini, "log", "console")) {
        nv_log_set_console(nv_ini_get_bool(ini, "log", "console", nv_log_get_console()));
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

    str = nv_ini_get_string(ini, "ipc", "mq_name", NULL);
    if (str) {
        nv_main_cfg_set_str(&ctx->mq_name_dup, str);
        ctx->opts.mq_name = ctx->mq_name_dup;
    }

    ctx->conf.daemon             = ctx->opts.daemon;
    ctx->conf.worker_processes   = ctx->opts.worker_threads;
    ctx->conf.worker_connections = ctx->opts.worker_connections;
    return NV_OK;
}

static int nv_main_log_init(nv_main_ctx_t *ctx)
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

static int nv_main_pidfile_create(nv_main_ctx_t *ctx)
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
        snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
        (void)write(fd, buf, strlen(buf));
    }

    ctx->pid_fd = fd;
    return NV_OK;
}

static void nv_main_pidfile_remove(nv_main_ctx_t *ctx)
{
    if (ctx->pid_fd >= 0) {
        close(ctx->pid_fd);
        ctx->pid_fd = -1;
    }
    if (ctx->opts.pid_file) {
        unlink(ctx->opts.pid_file);
    }
}

static void nv_main_on_signal(int signum)
{
    if (!g_main_ctx) {
        return;
    }

    switch (signum) {
    case SIGINT:
    case SIGTERM:
        g_main_ctx->quit = 1;
        nv_loop_stop(&g_main_ctx->loop);
        break;
    case SIGHUP:
        g_main_ctx->reload = 1;
        break;
    case SIGUSR1:
        g_main_ctx->restart = 1;
        g_main_ctx->quit = 1;
        nv_loop_stop(&g_main_ctx->loop);
        break;
    default:
        break;
    }
}

static int nv_main_signals_register(void)
{
    if (nv_signal_register(SIGINT,  nv_main_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGTERM, nv_main_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGHUP,  nv_main_on_signal) != 0) return NV_ERROR;
    if (nv_signal_register(SIGUSR1, nv_main_on_signal) != 0) return NV_ERROR;
    signal(SIGPIPE, SIG_IGN);
    return NV_OK;
}

int nv_main_startup_init(nv_main_ctx_t *ctx)
{
    int run_daemon;

    if (!ctx) {
        return NV_ERROR;
    }

    g_main_ctx = ctx;
    ctx->phase = NV_MAIN_PHASE_STARTUP;

    nv_main_exception_init();

    /* 命令行优先：前台模式先打开终端日志 */
    nv_log_set_console(ctx->opts.foreground || !ctx->opts.daemon);

    if (nv_main_load_config(ctx) != NV_OK) {
        return NV_ERROR;
    }

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

    if (nv_main_log_init(ctx) != NV_OK) {
        return NV_ERROR;
    }

    nv_log_set_module("MAIN");
    nv_log_notice("libnv %s, build time %s",
                  nv_version_string(), nv_build_time_string());
    nv_log_info("系统初始化完成 [%s]", nv_main_phase_name(ctx->phase));

    if (nv_main_pidfile_create(ctx) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_main_signals_register() != NV_OK) {
        nv_log_error("signal register failed");
        return NV_ERROR;
    }

    if (nv_core_init(&ctx->conf) != NV_OK) {
        nv_log_error("core init failed");
        return NV_ERROR;
    }

    return NV_OK;
}

int nv_main_business_init(nv_main_ctx_t *ctx)
{
    nv_loop_config_t lcfg = NV_LOOP_CONFIG_DEFAULT;

    if (!ctx) {
        return NV_ERROR;
    }

    ctx->phase = NV_MAIN_PHASE_BUSINESS;
    nv_log_info("business modules init [%s]", nv_main_phase_name(ctx->phase));

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

    ctx->thread_pool = nv_threadpool_create(ctx->opts.worker_threads, 256);
    if (!ctx->thread_pool) {
        nv_log_error("thread pool create failed");
        return NV_ERROR;
    }

    ctx->msg_queue = nv_init_message_queue(ctx->opts.mq_name, 64, MAX_MSG_SIZE, 0);
    if (!ctx->msg_queue) {
        nv_log_warning("message queue init skipped: %s", ctx->opts.mq_name);
    }

    return NV_OK;
}

void nv_main_business_cleanup(nv_main_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    nv_log_info("business modules cleanup");

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

void nv_main_exception_init(void)
{
    struct rlimit rl;

    memset(&rl, 0, sizeof(rl));
    rl.rlim_cur = RLIM_INFINITY;
    rl.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rl);

    signal(SIGSEGV, nv_main_exception_handler);
    signal(SIGABRT, nv_main_exception_handler);
    signal(SIGFPE,  nv_main_exception_handler);
    signal(SIGILL, nv_main_exception_handler);
}

void nv_main_exception_handler(int signum)
{
    if (g_main_ctx) {
        g_main_ctx->phase = NV_MAIN_PHASE_EXCEPTION;
    }

    nv_log_fatal("fatal signal caught: %d (%s)", signum, strsignal(signum));
    syslog(LOG_CRIT, "nv main fatal signal: %d", signum);

    if (g_main_ctx) {
        g_main_ctx->quit = 1;
        nv_loop_stop(&g_main_ctx->loop);
        nv_main_business_cleanup(g_main_ctx);
        nv_core_cleanup();
        nv_main_pidfile_remove(g_main_ctx);
        nv_log_close();
        if (g_main_ctx->opts.use_syslog) {
            closelog();
        }
        g_main_ctx = NULL;
    }

    signal(signum, SIG_DFL);
    raise(signum);
}

static void nv_main_idle_handler(nv_loop_t *loop, void *ev, void *data)
{
    nv_main_ctx_t *ctx = (nv_main_ctx_t *)data;
    (void)loop;
    (void)ev;

    if (!ctx) {
        return;
    }

    if (ctx->reload) {
        ctx->reload = 0;
        nv_log_info("reload config (SIGHUP)");
        nv_main_load_config(ctx);
        if (g_main_hooks && g_main_hooks->on_reload) {
            g_main_hooks->on_reload(ctx);
        }
    }

    if (g_main_hooks && g_main_hooks->on_idle) {
        g_main_hooks->on_idle(ctx);
    }
}

int nv_main_run_loop(nv_main_ctx_t *ctx, const nv_main_hooks_t *hooks)
{
    if (!ctx) {
        return NV_ERROR;
    }

    (void)hooks;
    ctx->phase = NV_MAIN_PHASE_LOOP;
    nv_log_info("enter main event loop [%s]", nv_main_phase_name(ctx->phase));

    memset(&g_idle_ev, 0, sizeof(g_idle_ev));
    g_idle_ev.type    = NV_EVENT_TYPE_IDLE;
    g_idle_ev.handler = nv_main_idle_handler;
    g_idle_ev.data    = ctx;
    nv_loop_add_idle(&ctx->loop, &g_idle_ev);

    return nv_loop_run(&ctx->loop);
}

void nv_main_shutdown(nv_main_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    if (ctx->phase == NV_MAIN_PHASE_SHUTDOWN) {
        return;
    }

    ctx->phase = NV_MAIN_PHASE_SHUTDOWN;
    nv_log_info("graceful shutdown [%s]", nv_main_phase_name(ctx->phase));

    nv_loop_stop(&ctx->loop);
    nv_core_cleanup();

    nv_main_pidfile_remove(ctx);
    nv_log_close();
    if (ctx->opts.use_syslog) {
        closelog();
    }

    nv_ini_free(ctx->ini);
    ctx->ini = NULL;
    free(ctx->pid_file_dup);
    free(ctx->log_file_dup);
    free(ctx->mq_name_dup);
    ctx->pid_file_dup = NULL;
    ctx->log_file_dup = NULL;
    ctx->mq_name_dup  = NULL;

    g_main_ctx = NULL;
    ctx->phase = NV_MAIN_PHASE_NONE;
}

int nv_main_run(nv_main_ctx_t *ctx, int argc, char **argv,
                const nv_main_hooks_t *hooks)
{
    int rc;

    if (!ctx) {
        return NV_ERROR;
    }

    g_main_hooks = hooks;

    rc = nv_main_parse_args(ctx, argc, argv);
    if (rc == NV_DECLINED) {
        return 0;
    }
    if (rc != NV_OK) {
        return NV_ERROR;
    }

    if (nv_main_startup_init(ctx) != NV_OK) {
        nv_main_shutdown(ctx);
        return NV_ERROR;
    }

    if (hooks && hooks->on_business_init) {
        if (hooks->on_business_init(ctx) != NV_OK) {
            nv_log_error("user business init failed");
            nv_main_business_cleanup(ctx);
            nv_main_shutdown(ctx);
            return NV_ERROR;
        }
    } else if (nv_main_business_init(ctx) != NV_OK) {
        nv_main_business_cleanup(ctx);
        nv_main_shutdown(ctx);
        return NV_ERROR;
    }

    nv_main_run_loop(ctx, hooks);

    if (hooks && hooks->on_business_cleanup) {
        hooks->on_business_cleanup(ctx);
    } else {
        nv_main_business_cleanup(ctx);
    }

    if (ctx->restart) {
        nv_log_info("restart requested (SIGUSR1), re-exec");
        nv_main_shutdown(ctx);
        execv(ctx->opts.prog_name, argv);
        return NV_ERROR;
    }

    nv_log_info("main process exiting");
    nv_main_shutdown(ctx);
    return NV_OK;
}
