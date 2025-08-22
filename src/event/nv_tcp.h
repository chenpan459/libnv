#ifndef _NV_EVENT_TCP_H_INCLUDED_
#define _NV_EVENT_TCP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_core.h>
#include <nv_event.h>
#include <nv_loop.h>

/* 前向声明 */
typedef struct nv_tcp_s nv_tcp_t;
typedef struct nv_tcp_server_s nv_tcp_server_t;
typedef struct nv_tcp_client_s nv_tcp_client_t;
typedef struct nv_tcp_buffer_s nv_tcp_buffer_t;
typedef struct nv_tcp_pool_s nv_tcp_pool_t;

/* TCP连接状态 */
typedef enum {
    NV_TCP_CLOSED = 0,
    NV_TCP_LISTENING,
    NV_TCP_CONNECTING,
    NV_TCP_CONNECTED,
    NV_TCP_CLOSING
} nv_tcp_state_t;

/* TCP连接结构 */
typedef struct nv_tcp_s {
    nv_handle_t handle;
    int socketfd;
    int port;
    int family;
    int type;
    int protocol;
    int flags;
    nv_tcp_state_t state;
    struct nv_loop_s *loop;
    nv_event_ext_t read_event;
    nv_event_ext_t write_event;
    nv_pool_t *pool;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    void *user_data;
} nv_tcp_t;

/* TCP服务器结构 */
typedef struct nv_tcp_server_s {
    nv_tcp_t *listener;
    struct nv_loop_s *loop;
    int port;
    int backlog;
    int max_connections;
    nv_pool_t *pool;
    void (*connection_handler)(nv_tcp_t *conn);
    void (*data_handler)(nv_tcp_t *conn, const char *data, size_t len);
    void (*close_handler)(nv_tcp_t *conn);
} nv_tcp_server_t;

/* TCP客户端结构 */
typedef struct nv_tcp_client_s {
    nv_tcp_t *connection;
    struct nv_loop_s *loop;
    char *server_ip;
    int server_port;
    int timeout_ms;
    nv_pool_t *pool;
    void (*connect_handler)(nv_tcp_client_t *client, int status);
    void (*data_handler)(nv_tcp_client_t *client, const char *data, size_t len);
    void (*close_handler)(nv_tcp_client_t *client);
} nv_tcp_client_t;

/* TCP基础API函数声明 */
int nv_tcp_init(struct nv_loop_s* loop, nv_tcp_t* tcp);
int nv_tcp_bind(nv_tcp_t* tcp, uint32_t port);
int nv_tcp_listen(nv_tcp_t* tcp, nv_int32 backlog);
nv_tcp_t* nv_tcp_accept(nv_tcp_t* tcp);
int nv_tcp_connect(nv_tcp_t* tcp, const nv_char* ip, nv_int32 port);
int nv_tcp_read(nv_tcp_t* tcp, nv_char *buff, nv_int32 size);
int nv_tcp_write(nv_tcp_t* tcp, nv_char *buff, nv_int32 size);
int nv_tcp_close(nv_tcp_t* tcp);

/* TCP服务器API */
int nv_tcp_server_create(nv_tcp_server_t *server, struct nv_loop_s *loop, int port);
int nv_tcp_server_start(nv_tcp_server_t *server);
int nv_tcp_server_stop(nv_tcp_server_t *server);
void nv_tcp_server_destroy(nv_tcp_server_t *server);

/* TCP客户端API */
int nv_tcp_client_create(nv_tcp_client_t *client, struct nv_loop_s *loop);
int nv_tcp_client_connect(nv_tcp_client_t *client, const char *ip, int port);
int nv_tcp_client_disconnect(nv_tcp_client_t *client);
void nv_tcp_client_destroy(nv_tcp_client_t *client);

/* TCP连接管理 */
int nv_tcp_set_nonblocking(nv_tcp_t *tcp);
int nv_tcp_set_keepalive(nv_tcp_t *tcp, int enable, int idle, int interval, int count);
int nv_tcp_set_nodelay(nv_tcp_t *tcp, int enable);
int nv_tcp_set_reuseaddr(nv_tcp_t *tcp, int enable);

/* TCP缓冲区管理 */
typedef struct nv_tcp_buffer_s {
    char *data;
    size_t size;
    size_t capacity;
    nv_pool_t *pool;
} nv_tcp_buffer_t;

int nv_tcp_buffer_create(nv_tcp_buffer_t *buf, nv_pool_t *pool, size_t initial_size);
int nv_tcp_buffer_write(nv_tcp_buffer_t *buf, const char *data, size_t len);
int nv_tcp_buffer_read(nv_tcp_buffer_t *buf, char *data, size_t len);
void nv_tcp_buffer_clear(nv_tcp_buffer_t *buf);
void nv_tcp_buffer_destroy(nv_tcp_buffer_t *buf);

/* TCP连接池管理 */
typedef struct nv_tcp_pool_s {
    nv_tcp_t **connections;
    int max_connections;
    int current_connections;
    int free_connections;
    nv_pool_t *pool;
    pthread_mutex_t mutex;
} nv_tcp_pool_t;

int nv_tcp_pool_create(nv_tcp_pool_t *pool, struct nv_loop_s *loop, int max_conns);
nv_tcp_t* nv_tcp_pool_get_connection(nv_tcp_pool_t *pool);
int nv_tcp_pool_return_connection(nv_tcp_pool_t *pool, nv_tcp_t *conn);
void nv_tcp_pool_destroy(nv_tcp_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif