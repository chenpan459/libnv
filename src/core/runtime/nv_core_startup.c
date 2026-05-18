/************************************************
 * nv_core_startup.c - 启动、PID、信号、资源限制
 ***********************************************/
#include "nv_core_private.h"
#include "nv_core_config.h"
#include "nv_core_lock.h"
#include "nv_version.h"
#include <nv_log.h>
#include <nv_base.h>
#include <nv_signal.h>
#include <nv_fork.h>
#include <nv_ini.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int nv_core_log_init(nv_core_ctx_t *ctx)
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

int nv_core_pidfile_create(nv_core_ctx_t *ctx)
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

void nv_core_pidfile_remove(nv_core_ctx_t *ctx)
{
    if (ctx->pid_fd >= 0) {
        close(ctx->pid_fd);
        ctx->pid_fd = -1;
    }
    if (ctx->opts.pid_file) {
        unlink(ctx->opts.pid_file);
    }
}

void nv_core_on_signal(int signum)
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

int nv_core_signals_register_loop(nv_core_ctx_t *ctx)
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

void nv_core_apply_rlimits(nv_core_ctx_t *ctx)
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

    if (nv_core_loadlibs_load(ctx) != NV_OK) {
        return NV_ERROR;
    }

    if (nv_base_init(&ctx->conf) != NV_OK) {
        nv_log_error("core init failed");
        return NV_ERROR;
    }

    return NV_OK;
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
    /*
     * 仅 async-signal-safe 操作：记录信号、请求退出、唤醒 epoll。
     * 日志与资源释放在主线程 nv_core_handle_fatal() 完成。
     */
    if (g_core_ctx) {
        g_core_ctx->fatal_signum = signum;
        g_core_ctx->quit         = 1;
        nv_loop_stop(&g_core_ctx->loop);
    }
}

void nv_core_handle_fatal(nv_core_ctx_t *ctx)
{
    int signum;

    if (!ctx || !ctx->fatal_signum) {
        return;
    }

    signum = (int)ctx->fatal_signum;
    ctx->phase = NV_CORE_PHASE_EXCEPTION;

    nv_log_fatal("fatal signal caught: %d (%s)", signum, strsignal(signum));

    ctx->fatal_signum = 0;
    signal(signum, SIG_DFL);
    raise(signum);
}
