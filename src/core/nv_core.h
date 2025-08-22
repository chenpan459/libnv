/************************************************
 * @文件名: nv_core.h
 * @功能: libnv核心层头文件，定义核心数据结构和接口
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义核心数据结构
 * 2024-11-04 - 完善核心层架构设计
 * 2024-11-04 - 集成内存池模块
 ***********************************************/

#ifndef _NV_CORE_H_INCLUDED_
#define _NV_CORE_H_INCLUDED_

#include <nv_config.h>

/* 核心状态返回值 */
#define NV_OK          0
#define NV_ERROR      -1
#define NV_AGAIN      -2
#define NV_BUSY       -3
#define NV_DONE       -4
#define NV_DECLINED   -5
#define NV_ABORT      -6

/* 基础句柄结构 */
typedef struct nv_handle_s {
    int type;
    int flags;
    void *private_data;
} nv_handle_t;

/* 核心模块类型 */
typedef enum {
    NV_MODULE_CORE = 0,
    NV_MODULE_EVENT,
    NV_MODULE_NETWORK,
    NV_MODULE_SYSTEM,
    NV_MODULE_UTILITY,
    NV_MODULE_MEMORY_POOL,  /* 新增内存池模块 */
    NV_MODULE_MAX
} nv_module_type_t;

/* 核心模块结构 */
typedef struct nv_module_s {
    nv_module_type_t type;
    const char *name;
    int (*init)(void *ctx);
    int (*cleanup)(void *ctx);
    void *private_data;
} nv_module_t;

/* 核心配置结构 */
typedef struct nv_conf_s {
    int daemon;
    int worker_processes;
    int worker_connections;
    char *pid_file;
    char *log_file;
    int log_level;
    void *modules[NV_MODULE_MAX];
} nv_conf_t;

/* 核心循环结构 */
typedef struct nv_cycle_s {
    nv_conf_t *conf;
    nv_module_t *modules[NV_MODULE_MAX];
    int module_count;
    int running;
    void *private_data;
} nv_cycle_t;

/* 内存池结构（前向声明） */
typedef struct nv_pool_s nv_pool_t;

/* 链表结构 */
typedef struct nv_chain_s {
    struct nv_chain_s *next;
    struct nv_chain_s *prev;
    void *data;
    size_t size;
} nv_chain_t;

/* 事件结构 */
typedef struct nv_event_s {
    int fd;
    int events;
    int revents;
    void (*handler)(struct nv_event_s *ev);
    void *data;
    int active;
} nv_event_t;

/* 连接结构 */
typedef struct nv_connection_s {
    int fd;
    struct sockaddr *sockaddr;
    socklen_t socklen;
    nv_event_t *read;
    nv_event_t *write;
    void *data;
} nv_connection_t;

/* 线程任务结构 */
typedef struct nv_thread_task_s {
    void (*handler)(void *data);
    void *data;
    struct nv_thread_task_s *next;
} nv_thread_task_t;

/* 函数指针类型定义 */
typedef void (*nv_event_handler_pt)(nv_event_t *ev);
typedef void (*nv_connection_handler_pt)(nv_connection_t *c);
typedef int (*nv_module_init_pt)(void *ctx);
typedef int (*nv_module_cleanup_pt)(void *ctx);

/* 核心API函数声明 */
int nv_core_init(nv_conf_t *conf);
int nv_core_cleanup(void);
nv_cycle_t* nv_core_create_cycle(nv_conf_t *conf);
int nv_core_run_cycle(nv_cycle_t *cycle);

/* 内存池API函数声明（前向声明） */
/* 注意：这些函数的具体实现和类型定义在 nv_pool.h 中 */
/* 这里只提供前向声明，避免循环依赖 */

/* 工具宏定义 */
#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

#define nv_abs(value)       (((value) >= 0) ? (value) : - (value))
#define nv_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define nv_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

/* 平台相关宏定义 */
#if (NV_HAVE_OPENAT)
#define NV_DISABLE_SYMLINKS_OFF        0
#define NV_DISABLE_SYMLINKS_ON         1
#define NV_DISABLE_SYMLINKS_NOTOWNER   2
#endif

/* 核心模块注册宏 */
#define NV_MODULE_REGISTER(type, name, init_func, cleanup_func) \
    static nv_module_t nv_module_##name = { \
        .type = type, \
        .name = #name, \
        .init = init_func, \
        .cleanup = cleanup_func, \
        .private_data = NULL \
    }; \
    __attribute__((constructor)) \
    void nv_module_##name##_register(void) { \
        nv_core_register_module(&nv_module_##name); \
    }

/* 内存池模块注册宏 */
#define NV_MEMORY_POOL_MODULE_REGISTER(name, init_func, cleanup_func) \
    NV_MODULE_REGISTER(NV_MODULE_MEMORY_POOL, name, init_func, cleanup_func)

/* 内部函数声明 */
int nv_core_register_module(nv_module_t *module);

void nv_cpuinfo(void);

#endif /* _NV_CORE_H_INCLUDED_ */
