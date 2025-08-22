#ifndef _NV_EVENT_UDP_H_INCLUDED_
#define _NV_EVENT_UDP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_core.h>
#include <nv_event.h>
#include <nv_loop.h>

/* 前向声明 */
typedef struct nv_udp_s nv_udp_t;
typedef struct nv_udp_server_s nv_udp_server_t;
typedef struct nv_udp_client_s nv_udp_client_t;
typedef struct nv_udp_buffer_s nv_udp_buffer_t;
typedef struct nv_udp_pool_s nv_udp_pool_t;

/* UDP连接状态 */
typedef enum {
    NV_UDP_CLOSED = 0,
    NV_UDP_BOUND,
    NV_UDP_CONNECTED,
    NV_UDP_MULTICAST
} nv_udp_state_t;

/* UDP连接结构 */
typedef struct nv_udp_s {
    nv_handle_t handle;
    int socketfd;
    int port;
    int family;
    int type;
    int protocol;
    int flags;
    nv_udp_state_t state;
    struct nv_loop_s *loop;
    nv_event_ext_t read_event;
    nv_event_ext_t write_event;
    nv_pool_t *pool;
    struct sockaddr_in src_addr;
    struct sockaddr_in dest_addr;
    struct sockaddr_in multicast_addr;
    void *user_data;
} nv_udp_t;

/* UDP服务器结构 */
typedef struct nv_udp_server_s {
    nv_udp_t *listener;
    struct nv_loop_s *loop;
    int port;
    int max_clients;
    nv_pool_t *pool;
    void (*data_handler)(nv_udp_t *udp, const char *data, size_t len, 
                         struct sockaddr_in *client_addr);
    void (*error_handler)(nv_udp_t *udp, int error);
} nv_udp_server_t;

/* UDP客户端结构 */
typedef struct nv_udp_client_s {
    nv_udp_t *connection;
    struct nv_loop_s *loop;
    char *server_ip;
    int server_port;
    int timeout_ms;
    nv_pool_t *pool;
    void (*data_handler)(nv_udp_client_t *client, const char *data, size_t len);
    void (*error_handler)(nv_udp_client_t *client, int error);
} nv_udp_client_t;

/* UDP基础API函数声明 */
int nv_udp_init(struct nv_loop_s* loop, nv_udp_t* udp);
int nv_udp_bind(nv_udp_t* udp, uint32_t port);
int nv_udp_connect(nv_udp_t* udp, const nv_char* ip, nv_int32 port);
int nv_udp_send(nv_udp_t* udp, const nv_char* buff, nv_int32 size);
int nv_udp_sendto(nv_udp_t* udp, const nv_char* buff, nv_int32 size, 
                   const struct sockaddr_in* dest_addr);
int nv_udp_recv(nv_udp_t* udp, nv_char* buff, nv_int32 size);
int nv_udp_recvfrom(nv_udp_t* udp, nv_char* buff, nv_int32 size, 
                     struct sockaddr_in* src_addr);
int nv_udp_close(nv_udp_t* udp);

/* UDP服务器API */
int nv_udp_server_create(nv_udp_server_t *server, struct nv_loop_s *loop, int port);
int nv_udp_server_start(nv_udp_server_t *server);
int nv_udp_server_stop(nv_udp_server_t *server);
void nv_udp_server_destroy(nv_udp_server_t *server);

/* UDP客户端API */
int nv_udp_client_create(nv_udp_client_t *client, struct nv_loop_s *loop);
int nv_udp_client_connect(nv_udp_client_t *client, const char *ip, int port);
int nv_udp_client_disconnect(nv_udp_client_t *client);
void nv_udp_client_destroy(nv_udp_client_t *client);

/* UDP多播支持 */
int nv_udp_join_multicast(nv_udp_t *udp, const char *multicast_ip, 
                           const char *interface_ip);
int nv_udp_leave_multicast(nv_udp_t *udp, const char *multicast_ip, 
                            const char *interface_ip);
int nv_udp_set_multicast_ttl(nv_udp_t *udp, int ttl);
int nv_udp_set_multicast_loopback(nv_udp_t *udp, int enable);

/* UDP广播支持 */
int nv_udp_set_broadcast(nv_udp_t *udp, int enable);
int nv_udp_send_broadcast(nv_udp_t *udp, const char *data, size_t len, int port);

/* UDP连接管理 */
int nv_udp_set_nonblocking(nv_udp_t *udp);
int nv_udp_set_reuseaddr(nv_udp_t *udp, int enable);
int nv_udp_set_reuseport(nv_udp_t *udp, int enable);

/* UDP缓冲区管理 */
typedef struct nv_udp_buffer_s {
    char *data;
    size_t size;
    size_t capacity;
    struct sockaddr_in addr;
    nv_pool_t *pool;
} nv_udp_buffer_t;

int nv_udp_buffer_create(nv_udp_buffer_t *buf, nv_pool_t *pool, size_t initial_size);
int nv_udp_buffer_write(nv_udp_buffer_t *buf, const char *data, size_t len);
int nv_udp_buffer_read(nv_udp_buffer_t *buf, char *data, size_t len);
void nv_udp_buffer_clear(nv_udp_buffer_t *buf);
void nv_udp_buffer_destroy(nv_udp_buffer_t *buf);

/* UDP连接池管理 */
typedef struct nv_udp_pool_s {
    nv_udp_t **connections;
    int max_connections;
    int current_connections;
    int free_connections;
    nv_pool_t *pool;
    pthread_mutex_t mutex;
} nv_udp_pool_t;

int nv_udp_pool_create(nv_udp_pool_t *pool, struct nv_loop_s *loop, int max_conns);
nv_udp_t* nv_udp_pool_get_connection(nv_udp_pool_t *pool);
int nv_udp_pool_return_connection(nv_udp_pool_t *pool, nv_udp_t *conn);
void nv_udp_pool_destroy(nv_udp_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif