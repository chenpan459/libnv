/************************************************
 * @文件名: nv_main.h
 * @功能: libnv 主进程框架（启动、业务初始化、事件循环、优雅退出、异常处理）
 * @作者: chenpan
 * @日期: 2026-05-17
 ***********************************************/

#ifndef _NV_MAIN_H_INCLUDED_
#define _NV_MAIN_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_core.h>
#include <nv_loop.h>
#include <nv_log.h>
#include <nv_fork.h>
#include <nv_signal.h>
#include <nv_thread_pool.h>
#include <nv_message_queue.h>
#include <nv_ini.h>

/* 默认路径 */
#define NV_MAIN_DEFAULT_CONFIG   "/etc/nv/nv.conf"
#define NV_MAIN_DEFAULT_PID_FILE "/var/run/nv.pid"
#define NV_MAIN_DEFAULT_LOG_FILE "/var/log/nv.log"

/* 主进程运行阶段 */
typedef enum {
    NV_MAIN_PHASE_NONE = 0,
    NV_MAIN_PHASE_STARTUP,      /* 启动初始化 */
    NV_MAIN_PHASE_BUSINESS,     /* 业务模块初始化 */
    NV_MAIN_PHASE_LOOP,         /* 主事件循环 */
    NV_MAIN_PHASE_SHUTDOWN,     /* 优雅退出 */
    NV_MAIN_PHASE_EXCEPTION     /* 异常处理 */
} nv_main_phase_t;

/* 命令行 / 运行时选项 */
typedef struct nv_main_opts_s {
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
    int         help;
    int         version;
} nv_main_opts_t;

/* 主进程上下文 */
typedef struct nv_main_ctx_s {
    nv_main_opts_t   opts;
    nv_conf_t        conf;
    nv_main_phase_t  phase;

    nv_loop_t        loop;
    threadpool_t    *thread_pool;
    message_queue_t *msg_queue;

    int              pid_fd;
    volatile sig_atomic_t quit;
    volatile sig_atomic_t reload;
    volatile sig_atomic_t restart;

    nv_ini_t        *ini;           /* 已解析的 INI 配置 */
    char            *pid_file_dup;
    char            *log_file_dup;
    char            *mq_name_dup;

    void            *user_data;
} nv_main_ctx_t;

/* 业务扩展钩子 */
typedef struct nv_main_hooks_s {
    int  (*on_business_init)(nv_main_ctx_t *ctx);
    void (*on_business_cleanup)(nv_main_ctx_t *ctx);
    void (*on_reload)(nv_main_ctx_t *ctx);
    void (*on_idle)(nv_main_ctx_t *ctx);
} nv_main_hooks_t;

/* ---------- 1. 启动初始化 ---------- */
int  nv_main_parse_args(nv_main_ctx_t *ctx, int argc, char **argv);
int  nv_main_load_config(nv_main_ctx_t *ctx);
int  nv_main_startup_init(nv_main_ctx_t *ctx);

/* ---------- 2. 业务模块初始化 ---------- */
int  nv_main_business_init(nv_main_ctx_t *ctx);
void nv_main_business_cleanup(nv_main_ctx_t *ctx);

/* ---------- 3. 主事件循环 ---------- */
int  nv_main_run_loop(nv_main_ctx_t *ctx, const nv_main_hooks_t *hooks);

/* ---------- 4. 优雅退出 ---------- */
void nv_main_shutdown(nv_main_ctx_t *ctx);

/* ---------- 5. 异常处理 ---------- */
void nv_main_exception_init(void);
void nv_main_exception_handler(int signum);

/* ---------- 统一入口 ---------- */
int  nv_main_run(nv_main_ctx_t *ctx, int argc, char **argv,
                 const nv_main_hooks_t *hooks);

/* 工具 */
const char *nv_main_phase_name(nv_main_phase_t phase);
void        nv_main_usage(const char *prog);

#ifdef __cplusplus
}
#endif

#endif /* _NV_MAIN_H_INCLUDED_ */
