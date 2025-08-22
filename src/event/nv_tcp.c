#include "nv_tcp.h"
#include <nv_socket.h>
#include <nv_mem.h>

int nv_tcp_init(struct nv_loop_s* loop, nv_tcp_t* tcp){
    tcp->socketfd = nv_tcp_socket_create();
    if(tcp->socketfd < 0){
        return -1;
    }
    tcp->state = NV_TCP_CLOSED;
    tcp->loop = loop;
    return 0;
}

int nv_tcp_bind(nv_tcp_t* tcp, uint32_t port){
    if(nv_socket_bind(tcp->socketfd, port) < 0){
        nv_socket_close(tcp->socketfd);
        return -1;
    }
    tcp->port = port;
    return 0;
}

int nv_tcp_listen(nv_tcp_t* tcp, nv_int32 backlog){
    if(nv_tcp_socket_listen(tcp->socketfd, backlog) < 0){
        nv_socket_close(tcp->socketfd);
        return -1;
    }
    tcp->state = NV_TCP_LISTENING;
    return 0;
}

nv_tcp_t* nv_tcp_accept(nv_tcp_t* tcp){
    nv_tcp_t *client = nv_malloc(sizeof(nv_tcp_t));
    if(!client) {
        return NULL;
    }
    
    client->socketfd = nv_tcp_socket_accept(tcp->socketfd);
    if(client->socketfd < 0) {
        nv_free(client);
        return NULL;
    }
    
    client->state = NV_TCP_CONNECTED;
    client->loop = tcp->loop;
    
    return client;
}

int nv_tcp_connect(nv_tcp_t* tcp, const nv_char* ip, nv_int32 port){
    tcp->state = NV_TCP_CONNECTING;
    
    if(nv_tcp_socket_connect(tcp->socketfd, ip, port) < 0){
        nv_socket_close(tcp->socketfd);
        tcp->state = NV_TCP_CLOSED;
        return -1;
    }
    
    tcp->state = NV_TCP_CONNECTED;
    return 0;
}

int nv_tcp_read(nv_tcp_t* tcp, nv_char *buff, nv_int32 size){
    return nv_socket_recv(tcp->socketfd, buff, size, 0);
}

int nv_tcp_write(nv_tcp_t* tcp, nv_char *buff, nv_int32 size){
    return nv_socket_send(tcp->socketfd, buff, size, 0);
}

int nv_tcp_close(nv_tcp_t* tcp) {
    if (!tcp) return -1;
    
    if (tcp->socketfd != -1) {
        close(tcp->socketfd);
        tcp->socketfd = -1;
    }
    
    tcp->state = NV_TCP_CLOSED;
    return 0;
}

/* TCP服务器相关函数 */
int nv_tcp_server_create(nv_tcp_server_t *server, struct nv_loop_s *loop, int port) {
    if (!server || !loop) return -1;
    
    memset(server, 0, sizeof(nv_tcp_server_t));
    server->loop = loop;
    server->port = port;
    server->backlog = SOMAXCONN;
    server->max_connections = 1000;
    
    /* 创建监听socket */
    server->listener = malloc(sizeof(nv_tcp_t));
    if (!server->listener) return -1;
    
    memset(server->listener, 0, sizeof(nv_tcp_t));
    server->listener->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listener->socketfd == -1) {
        perror("NV: socket creation failed");
        free(server->listener);
        return -1;
    }
    
    /* 设置socket选项 */
    int opt = 1;
    setsockopt(server->listener->socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* 绑定地址 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(server->listener->socketfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("NV: bind failed");
        close(server->listener->socketfd);
        free(server->listener);
        return -1;
    }
    
    return 0;
}

int nv_tcp_server_start(nv_tcp_server_t *server) {
    if (!server || !server->listener || server->listener->socketfd == -1) return -1;
    
    if (listen(server->listener->socketfd, server->backlog) == -1) {
        perror("NV: listen failed");
        return -1;
    }
    
    server->listener->state = NV_TCP_LISTENING;
    
    /* 创建接受连接的事件 */
    nv_event_ext_t *accept_event = malloc(sizeof(nv_event_ext_t));
    if (!accept_event) return -1;
    
    /* 手动初始化事件结构 */
    memset(accept_event, 0, sizeof(nv_event_ext_t));
    accept_event->fd = server->listener->socketfd;
    accept_event->events = NV_EVENT_READ;
    accept_event->data = server;
    accept_event->type = NV_EVENT_TYPE_IO;
    accept_event->priority = NV_EVENT_PRIO_NORMAL;
    
    /* 添加到事件循环 */
    if (nv_loop_add_event(server->loop, accept_event, NV_EVENT_READ) != 0) {
        free(accept_event);
        return -1;
    }
    
    /* 保存事件引用 */
    server->listener->read_event = *accept_event;
    free(accept_event);
    return 0;
}

int nv_tcp_server_stop(nv_tcp_server_t *server) {
    if (!server) return -1;
    
    if (server->listener && server->listener->socketfd != -1) {
        close(server->listener->socketfd);
        server->listener->socketfd = -1;
        server->listener->state = NV_TCP_CLOSED;
    }
    
    if (server->listener) {
        free(server->listener);
        server->listener = NULL;
    }
    
    return 0;
}

/* TCP客户端相关函数 */
int nv_tcp_client_create(nv_tcp_client_t *client, struct nv_loop_s *loop) {
    if (!client || !loop) return -1;
    
    memset(client, 0, sizeof(nv_tcp_client_t));
    client->loop = loop;
    client->timeout_ms = 5000; /* 默认5秒超时 */
    
    return 0;
}

int nv_tcp_client_connect(nv_tcp_client_t *client, const char *ip, int port) {
    if (!client || !ip) return -1;
    
    /* 创建连接结构 */
    client->connection = malloc(sizeof(nv_tcp_t));
    if (!client->connection) return -1;
    
    memset(client->connection, 0, sizeof(nv_tcp_t));
    
    /* 创建socket */
    client->connection->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->connection->socketfd == -1) {
        perror("NV: socket creation failed");
        free(client->connection);
        return -1;
    }
    
    /* 连接服务器 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("NV: invalid IP address");
        close(client->connection->socketfd);
        free(client->connection);
        return -1;
    }
    
    if (connect(client->connection->socketfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("NV: connect failed");
        close(client->connection->socketfd);
        free(client->connection);
        return -1;
    }
    
    client->connection->state = NV_TCP_CONNECTED;
    client->server_ip = strdup(ip);
    client->server_port = port;
    return 0;
}

int nv_tcp_client_disconnect(nv_tcp_client_t *client) {
    if (!client) return -1;
    
    if (client->connection) {
        if (client->connection->socketfd != -1) {
            close(client->connection->socketfd);
            client->connection->socketfd = -1;
        }
        client->connection->state = NV_TCP_CLOSED;
        free(client->connection);
        client->connection = NULL;
    }
    
    if (client->server_ip) {
        free(client->server_ip);
        client->server_ip = NULL;
    }
    
    return 0;
}

/* TCP选项设置函数 */
int nv_tcp_set_nonblocking(nv_tcp_t *tcp) {
    if (!tcp || tcp->socketfd == -1) return -1;
    
    int flags = fcntl(tcp->socketfd, F_GETFL, 0);
    if (flags == -1) return -1;
    
    return fcntl(tcp->socketfd, F_SETFL, flags | O_NONBLOCK);
}

int nv_tcp_set_keepalive(nv_tcp_t *tcp, int enable, int idle, int interval, int count) {
    if (!tcp || tcp->socketfd == -1) return -1;
    
    if (setsockopt(tcp->socketfd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable)) == -1) {
        return -1;
    }
    
    if (enable) {
        setsockopt(tcp->socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
        setsockopt(tcp->socketfd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
        setsockopt(tcp->socketfd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
    }
    
    return 0;
}

int nv_tcp_set_nodelay(nv_tcp_t *tcp, int enable) {
    if (!tcp || tcp->socketfd == -1) return -1;
    
    return setsockopt(tcp->socketfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

int nv_tcp_set_reuseaddr(nv_tcp_t *tcp, int enable) {
    if (!tcp || tcp->socketfd == -1) return -1;
    
    return setsockopt(tcp->socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
}

/* TCP缓冲区相关函数 */
int nv_tcp_buffer_create(nv_tcp_buffer_t *buf, nv_pool_t *pool, size_t initial_size) {
    if (!buf) return -1;
    
    buf->data = malloc(initial_size);
    if (!buf->data) return -1;
    
    buf->size = 0;
    buf->capacity = initial_size;
    buf->pool = pool;
    
    return 0;
}

int nv_tcp_buffer_write(nv_tcp_buffer_t *buf, const char *data, size_t len) {
    if (!buf || !data) return -1;
    
    /* 检查缓冲区空间 */
    if (buf->size + len > buf->capacity) {
        /* 扩展缓冲区 */
        size_t new_capacity = buf->capacity * 2;
        if (new_capacity < buf->size + len) {
            new_capacity = buf->size + len;
        }
        
        char *new_data = realloc(buf->data, new_capacity);
        if (!new_data) return -1;
        
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    
    /* 写入数据 */
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    
    return 0;
}

int nv_tcp_buffer_read(nv_tcp_buffer_t *buf, char *data, size_t len) {
    if (!buf || !data) return -1;
    
    if (len > buf->size) {
        len = buf->size;
    }
    
    if (len > 0) {
        memcpy(data, buf->data, len);
        /* 移动剩余数据到开头 */
        if (len < buf->size) {
            memmove(buf->data, buf->data + len, buf->size - len);
        }
        buf->size -= len;
    }
    
    return len;
}

/* TCP连接池相关函数 */
int nv_tcp_pool_create(nv_tcp_pool_t *pool, struct nv_loop_s *loop, int max_conns) {
    if (!pool) return -1;
    
    pool->connections = malloc(max_conns * sizeof(nv_tcp_t*));
    if (!pool->connections) return -1;
    
    pool->max_connections = max_conns;
    pool->current_connections = 0;
    pool->free_connections = max_conns;
    
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        free(pool->connections);
        return -1;
    }
    
    return 0;
}

int nv_tcp_pool_return_connection(nv_tcp_pool_t *pool, nv_tcp_t *conn) {
    if (!pool || !conn) return -1;
    
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->free_connections > 0) {
        pool->connections[pool->free_connections - 1] = conn;
        pool->free_connections--;
        pool->current_connections++;
        pthread_mutex_unlock(&pool->mutex);
        return 0;
    }
    
    pthread_mutex_unlock(&pool->mutex);
    return -1;
}