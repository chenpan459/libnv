/************************************************
 * @文件名: nv.h
 * @功能: libnv主头文件，整合所有模块接口
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本接口
 * 2024-11-04 - 完善主头文件架构设计
 ***********************************************/

#ifndef _NV_H_INCLUDED_
#define _NV_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_core.h>
#include <nv_event.h>
#include <nv_loop.h>

/* 状态返回值 */
#define NV_SUCC 0        /* 成功 */
#define NV_FAIL -1       /* 失败 */
#define NV_EINVAL -2     /* 无效参数 */
#define NV_ENOMEM -3     /* 内存不足 */
#define NV_EIO -4        /* I/O 错误 */
#define NV_EAGAIN -5     /* 资源暂时不可用 */
#define NV_ECONNREFUSED -6 /* 连接被拒绝 */

/* 基础句柄结构 */
struct nv_handle_s {
    int type;
    int flags;
    void *private_data;
};

/* 基础TCP结构 */
struct nv_tcp_s {
    struct nv_handle_s handle;
    int socketfd;
    int port;
    int family;
    int type;
    int protocol;
    int flags;
};

/* 基础UDP结构 */
struct nv_udp_s {
    struct nv_handle_s handle;
    int socketfd;
    int port;
    int family;
    int type;
    int protocol;
    int flags;
    struct sockaddr_in src_addr;
    struct sockaddr_in dest_addr;
};

/* 基础事件循环结构 */
struct nv_loop_s {
    int epoll_fd;
    int max_events;
    struct epoll_event *events;
    int event_count;
    int running;
    nv_pool_t *pool;
    void *private_data;
};

/* 类型定义 */
typedef struct nv_handle_s nv_handle_t;
typedef struct nv_tcp_s    nv_tcp_t;
typedef struct nv_loop_s   nv_loop_t;
typedef struct nv_udp_s    nv_udp_t;

/* 库初始化/清理函数 */
int nv_library_init(void);
void nv_library_cleanup(void);
const char* nv_library_version(void);

/* 全局配置函数 */
int nv_global_config_set(const char *key, const char *value);
const char* nv_global_config_get(const char *key);
int nv_global_config_load(const char *config_file);

/* 错误处理函数 */
const char* nv_strerror(int error_code);
void nv_set_error_handler(void (*handler)(int error_code, const char *error_msg));

/* 内存管理函数 */
void* nv_malloc(size_t size);
void* nv_calloc(size_t nmemb, size_t size);
void* nv_realloc(void *ptr, size_t size);
void nv_free(void *ptr);

/* 日志函数 */
void nv_log_set_level(int level);
void nv_log_write(int level, const char *file, int line, const char *func, 
                  const char *format, ...);
void nv_log_set_output(FILE *output);

/* 日志级别定义 */
#define NV_LOG_EMERG   0   /* 系统不可用 */
#define NV_LOG_ALERT   1   /* 必须立即采取行动 */
#define NV_LOG_CRIT    2   /* 严重情况 */
#define NV_LOG_ERR     3   /* 错误情况 */
#define NV_LOG_WARN    4   /* 警告情况 */
#define NV_LOG_NOTICE  5   /* 正常但重要的情况 */
#define NV_LOG_INFO    6   /* 信息性消息 */
#define NV_LOG_DEBUG   7   /* 调试级别消息 */

/* 日志宏定义 */
#define nv_log_emerg(...)   nv_log_write(NV_LOG_EMERG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_alert(...)   nv_log_write(NV_LOG_ALERT, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_crit(...)    nv_log_write(NV_LOG_CRIT, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_error(...)   nv_log_write(NV_LOG_ERR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_warn(...)    nv_log_write(NV_LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_notice(...)  nv_log_write(NV_LOG_NOTICE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_info(...)    nv_log_write(NV_LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define nv_log_debug(...)   nv_log_write(NV_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)

/* 工具函数 */
unsigned long nv_get_tick_count(void);
void nv_sleep_ms(unsigned long milliseconds);
int nv_get_cpu_count(void);
int nv_get_memory_info(size_t *total, size_t *free, size_t *available);

/* 字符串工具函数 */
char* nv_strdup(const char *str);
char* nv_strndup(const char *str, size_t n);
int nv_strcasecmp(const char *s1, const char *s2);
int nv_strncasecmp(const char *s1, const char *s2, size_t n);

/* 时间工具函数 */
time_t nv_time(void);
struct tm* nv_localtime(const time_t *timep);
char* nv_strftime(const char *format, const struct tm *tm);
unsigned long nv_milliseconds(void);

/* 文件工具函数 */
int nv_file_exists(const char *path);
int nv_file_size(const char *path, size_t *size);
int nv_file_read(const char *path, char **data, size_t *size);
int nv_file_write(const char *path, const char *data, size_t size);
int nv_file_append(const char *path, const char *data, size_t size);

/* 目录工具函数 */
int nv_dir_exists(const char *path);
int nv_dir_create(const char *path, mode_t mode);
int nv_dir_remove(const char *path);
int nv_dir_list(const char *path, char ***entries, int *count);

/* 进程工具函数 */
pid_t nv_get_pid(void);
pid_t nv_get_ppid(void);
int nv_daemonize(void);
int nv_set_process_title(const char *title);

/* 信号工具函数 */
int nv_signal_set(int signo, void (*handler)(int));
int nv_signal_ignore(int signo);
int nv_signal_default(int signo);

/* 线程工具函数 */
int nv_thread_create(pthread_t *thread, void *(*start_routine)(void*), void *arg);
int nv_thread_join(pthread_t thread, void **retval);
int nv_thread_detach(pthread_t thread);
pthread_t nv_thread_self(void);

/* 互斥锁工具函数 */
int nv_mutex_init(pthread_mutex_t *mutex);
int nv_mutex_lock(pthread_mutex_t *mutex);
int nv_mutex_trylock(pthread_mutex_t *mutex);
int nv_mutex_unlock(pthread_mutex_t *mutex);
int nv_mutex_destroy(pthread_mutex_t *mutex);

/* 条件变量工具函数 */
int nv_cond_init(pthread_cond_t *cond);
int nv_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int nv_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, 
                      const struct timespec *abstime);
int nv_cond_signal(pthread_cond_t *cond);
int nv_cond_broadcast(pthread_cond_t *cond);
int nv_cond_destroy(pthread_cond_t *cond);

/* 信号量工具函数 */
int nv_sem_init(sem_t *sem, int pshared, unsigned int value);
int nv_sem_wait(sem_t *sem);
int nv_sem_trywait(sem_t *sem);
int nv_sem_post(sem_t *sem);
int nv_sem_getvalue(sem_t *sem, int *sval);
int nv_sem_destroy(sem_t *sem);

/* 共享内存工具函数 */
void* nv_shm_create(const char *name, size_t size, mode_t mode);
void* nv_shm_open(const char *name, int flags, mode_t mode);
int nv_shm_unlink(const char *name);
int nv_shm_close(void *addr);

/* 消息队列工具函数 */
mqd_t nv_mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int nv_mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
ssize_t nv_mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int nv_mq_close(mqd_t mqdes);
int nv_mq_unlink(const char *name);

/* 管道工具函数 */
int nv_pipe_create(int pipefd[2]);
int nv_pipe_read(int fd, char *buf, size_t count);
int nv_pipe_write(int fd, const char *buf, size_t count);

/* 网络工具函数 */
int nv_get_local_ip(char *ip, size_t size);
int nv_get_hostname(char *hostname, size_t size);
int nv_resolve_hostname(const char *hostname, char *ip, size_t size);
int nv_is_valid_ip(const char *ip);

/* 加密工具函数 */
int nv_md5_hash(const char *data, size_t len, char *hash);
int nv_sha1_hash(const char *data, size_t len, char *hash);
int nv_sha256_hash(const char *data, size_t len, char *hash);
int nv_base64_encode(const char *data, size_t len, char *encoded, size_t *encoded_len);
int nv_base64_decode(const char *encoded, size_t encoded_len, char *data, size_t *len);

/* 压缩工具函数 */
int nv_compress(const char *data, size_t len, char *compressed, size_t *compressed_len);
int nv_decompress(const char *compressed, size_t compressed_len, char *data, size_t *len);

/* 配置解析工具函数 */
int nv_config_parse(const char *config_file, void (*callback)(const char *key, const char *value));
int nv_config_get_int(const char *key, int default_value);
const char* nv_config_get_string(const char *key, const char *default_value);

/* 性能监控工具函数 */
typedef struct nv_perf_counter_s {
    const char *name;
    unsigned long count;
    unsigned long total_time;
    unsigned long min_time;
    unsigned long max_time;
    unsigned long start_time;
} nv_perf_counter_t;

void nv_perf_start(nv_perf_counter_t *counter);
void nv_perf_stop(nv_perf_counter_t *counter);
void nv_perf_reset(nv_perf_counter_t *counter);
void nv_perf_print(nv_perf_counter_t *counter);

/* 性能计数器宏 */
#define NV_PERF_COUNTER(name) \
    static nv_perf_counter_t nv_perf_##name = { #name, 0, 0, 0, 0, 0 }; \
    nv_perf_start(&nv_perf_##name)

#define NV_PERF_END(name) \
    nv_perf_stop(&nv_perf_##name)

/* 调试工具函数 */
void nv_debug_break(void);
void nv_debug_print(const char *format, ...);
void nv_debug_dump_memory(const void *data, size_t size);

/* 调试宏 */
#ifdef NV_DEBUG
#define nv_debug(...) nv_debug_print(__VA_ARGS__)
#define nv_assert(expr) do { if (!(expr)) { nv_debug_break(); } } while(0)
#else
#define nv_debug(...) do {} while(0)
#define nv_assert(expr) do {} while(0)
#endif

/* 版本信息 */
#define NV_VERSION_MAJOR 1
#define NV_VERSION_MINOR 0
#define NV_VERSION_PATCH 0
#define NV_VERSION_STRING "1.0.0"

#ifdef __cplusplus
}
#endif

#endif /* _NV_H_INCLUDED_ */