/************************************************
 * @文件名: nv_core.h
 * @功能: libnv 主进程框架（启动、业务初始化、事件循环、优雅退出、异常处理）
 * @作者: chenpan
 * @日期: 2026-05-17
 ***********************************************/

#ifndef _NV_CORE_H_INCLUDED_
#define _NV_CORE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_base.h>
#include <nv_event.h>
#include <nv_loop.h>
#include <nv_log.h>
#include <nv_fork.h>
#include <nv_signal.h>
#include <nv_thread_pool.h>
#include <nv_message_queue.h>
#include <nv_ini.h>
#include <pthread.h>
#include <time.h>

/* systemd notify：-1 自动检测 NOTIFY_SOCKET */
#define NV_CORE_SYSTEMD_AUTO  (-1)

#define NV_CORE_LOADLIB_INIT_SYMBOL     "nv_loadlib_init"
#define NV_CORE_LOADLIB_CLEANUP_SYMBOL  "nv_loadlib_cleanup"

/* 默认路径 */
#define NV_CORE_DEFAULT_CONFIG      "/etc/nv/nv.conf"
#define NV_CORE_DEFAULT_PID_FILE    "/var/run/nv.pid"
#define NV_CORE_DEFAULT_LOG_FILE    "/var/log/nv.log"
#define NV_CORE_DEFAULT_CTL_SOCKET  "/var/run/nv.ctl.sock"
#define NV_CORE_DEFAULT_LOCK_NAME   "libnv"
#define NV_CORE_DEFAULT_TELNET_PORT   2323
#define NV_CORE_DEFAULT_TELNET_BIND "127.0.0.1"

/* 主进程运行阶段 */
typedef enum {
    NV_CORE_PHASE_NONE = 0,
    NV_CORE_PHASE_STARTUP,      /* 启动初始化 */
    NV_CORE_PHASE_BUSINESS,     /* 业务模块初始化 */
    NV_CORE_PHASE_LOOP,         /* 主事件循环 */
    NV_CORE_PHASE_SHUTDOWN,     /* 优雅退出 */
    NV_CORE_PHASE_EXCEPTION     /* 异常处理 */
} nv_core_phase_t;

/* 命令行 / 运行时选项 */
typedef struct nv_core_opts_s {
    const char *prog_name;
    const char *config_file;
    const char *pid_file;
    const char *log_file;
    const char *mq_name;
    int         daemon;          /* 1: 后台守护进程 */
    int         foreground;      /* -f: 强制前台 */
    int         use_syslog;      /* 1: 同时写 syslog */
    int         log_level;
    int         worker_threads;
    int         worker_connections;
    int         worker_processes;        /* fork worker 数，0/1=单进程 */
    int         worker_respawn;          /* worker 崩溃是否拉起 */
    int         heartbeat_interval_sec;  /* 0=关闭周期心跳 */
    int         shutdown_timeout_sec;    /* 优雅退出最长等待秒数 */
    int         systemd_notify;          /* NV_CORE_SYSTEMD_AUTO / 0 / 1 */
    int         max_open_files;          /* RLIMIT_NOFILE，0=不修改 */
    long long   core_limit;              /* RLIMIT_CORE 字节，0=禁用，-1=无限 */
    const char *ctl_socket;              /* Unix 控制套接字路径 */
    const char *instance_lock;           /* 抽象锁名称 */
    int         telnet_enable;           /* Telnet CLI */
    int         telnet_port;
    const char *telnet_bind;
    const char *cli_username;
    const char *cli_password;
    int         help;
    int         version;
} nv_core_opts_t;

typedef struct nv_core_ctx_s nv_core_ctx_t;
typedef struct nv_core_pubsub_s nv_core_pubsub_t;

typedef int  (*nv_core_loadlib_init_pt)(nv_core_ctx_t *ctx, const char *args);
typedef void (*nv_core_loadlib_cleanup_pt)(nv_core_ctx_t *ctx);
typedef void (*nv_core_pubsub_handler_pt)(nv_core_ctx_t *ctx,
                                          const char *topic,
                                          const void *data,
                                          size_t len,
                                          void *user_data);

typedef struct nv_core_loadlib_s {
    char                         *path;
    char                         *args;
    void                         *handle;
    nv_core_loadlib_init_pt       init;
    nv_core_loadlib_cleanup_pt    cleanup;
    nv_core_ctx_t                *ctx;
    pthread_t                     thread;
    int                           thread_started;
    int                           thread_exited;
    int                           init_rc;
    struct nv_core_loadlib_s     *next;
} nv_core_loadlib_t;

/* 主进程上下文 */
struct nv_core_ctx_s {
    nv_core_opts_t   opts;
    nv_conf_t        conf;
    nv_core_phase_t  phase;

    nv_loop_t        loop;
    threadpool_t    *thread_pool;
    message_queue_t *msg_queue;

    int              pid_fd;
    volatile sig_atomic_t quit;
    volatile sig_atomic_t reload;
    volatile sig_atomic_t restart;
    volatile sig_atomic_t fatal_signum;  /* 致命信号号，主线程处理 */

    time_t           started_at;
    time_t           shutdown_started_at;
    nv_event_ext_t   heartbeat_ev;
    int              health_inited;

    int              instance_lock_fd;
    int              ctl_listen_fd;
    int              ctl_inited;
    nv_event_ext_t   ctl_listen_ev;

    int              telnet_listen_fd;
    int              telnet_inited;
    nv_event_ext_t   telnet_listen_ev;

    int              is_master;
    int              is_worker;
    int              worker_id;

    nv_ini_t        *ini;           /* 已解析的 INI 配置 */
    char            *pid_file_dup;
    char            *log_file_dup;
    char            *mq_name_dup;
    char            *ctl_socket_dup;
    char            *instance_lock_dup;
    char            *telnet_bind_dup;
    char            *cli_username_dup;
    char            *cli_password_dup;
    nv_core_loadlib_t *loadlibs;
    int              loadlibs_loaded;
    nv_core_pubsub_t *pubsubs;
    pthread_mutex_t  pubsub_mutex;
    pthread_cond_t   pubsub_cond;
    int              pubsub_inited;
    unsigned long    pubsub_next_id;

    void            *user_data;
};

/* 业务扩展钩子 */
typedef struct nv_core_hooks_s {
    int  (*on_business_init)(nv_core_ctx_t *ctx);
    void (*on_business_cleanup)(nv_core_ctx_t *ctx);
    void (*on_reload)(nv_core_ctx_t *ctx);
    void (*on_idle)(nv_core_ctx_t *ctx);
} nv_core_hooks_t;

/* ---------- 1. 启动初始化 ---------- */
int  nv_core_parse_args(nv_core_ctx_t *ctx, int argc, char **argv);
int  nv_core_load_config(nv_core_ctx_t *ctx);
int  nv_core_startup_init(nv_core_ctx_t *ctx);

/* ---------- 2. 业务模块初始化 ---------- */
int  nv_core_business_init(nv_core_ctx_t *ctx);
void nv_core_business_cleanup(nv_core_ctx_t *ctx);

/* ---------- 3. 主事件循环 ---------- */
int  nv_core_run_loop(nv_core_ctx_t *ctx, const nv_core_hooks_t *hooks);

/* ---------- 4. 优雅退出 ---------- */
void nv_core_shutdown(nv_core_ctx_t *ctx);

/* ---------- 5. 异常处理 ---------- */
void nv_core_exception_init(void);
void nv_core_exception_handler(int signum);
void nv_core_handle_fatal(nv_core_ctx_t *ctx);

/* ---------- 统一入口 ---------- */
int  nv_core_run(nv_core_ctx_t *ctx, int argc, char **argv,
                 const nv_core_hooks_t *hooks);

/* 运行时 */
int  nv_core_request_quit(nv_core_ctx_t *ctx);
int  nv_core_is_quitting(const nv_core_ctx_t *ctx);

/* 动态业务模块通信：进程内发布/订阅 */
int  nv_core_pubsub_subscribe(nv_core_ctx_t *ctx, const char *topic,
                              nv_core_pubsub_handler_pt handler,
                              void *user_data, void **sub_handle);
int  nv_core_pubsub_unsubscribe(nv_core_ctx_t *ctx, void *sub_handle);
int  nv_core_pubsub_publish(nv_core_ctx_t *ctx, const char *topic,
                            const void *data, size_t len);

/* 工具 */
const char *nv_core_phase_name(nv_core_phase_t phase);
void        nv_core_usage(const char *prog);

#ifdef __cplusplus
}
#endif

#endif /* _NV_CORE_H_INCLUDED_ */
