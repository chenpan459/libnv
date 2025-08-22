#include "nv_udp.h"
#include <nv_socket.h>
#include <nv_mem.h>

int nv_udp_init(struct nv_loop_s* loop, nv_udp_t* udp){
    udp->socketfd = nv_udp_socket_create();
    if(udp->socketfd < 0){
        return -1;
    }
    return 0; 
}

int nv_udp_bind(nv_udp_t* udp, uint32_t port){
    if(nv_socket_bind(udp->socketfd, port) < 0){
        nv_socket_close(udp->socketfd);
        return -1;
    }
     return 0; 
}

int nv_udp_connect(nv_udp_t* udp, const nv_char* ip, nv_int32 port){
    // UDP连接实现
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest_addr.sin_addr);
    
    if(connect(udp->socketfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
        return -1;
    }
    
    udp->dest_addr = dest_addr;
    udp->state = NV_UDP_CONNECTED;
    return 0;
}

int nv_udp_send(nv_udp_t* udp, const nv_char* buff, nv_int32 size){
    if(udp->state == NV_UDP_CONNECTED){
        return send(udp->socketfd, buff, size, 0);
    } else {
        return -1; // 需要先连接
    }
}

int nv_udp_sendto(nv_udp_t* udp, const nv_char* buff, nv_int32 size, 
                   const struct sockaddr_in* dest_addr){
    return nv_udp_socket_sendto(udp->socketfd, buff, size, dest_addr);
}

int nv_udp_recv(nv_udp_t* udp, nv_char* buff, nv_int32 size){
    if(udp->state == NV_UDP_CONNECTED){
        return recv(udp->socketfd, buff, size, 0);
    } else {
        return -1; // 需要先连接
    }
}

int nv_udp_recvfrom(nv_udp_t* udp, nv_char* buff, nv_int32 size, 
                     struct sockaddr_in* src_addr){
    return nv_udp_socket_recvfrom(udp->socketfd, buff, size, src_addr);
}

int nv_udp_close(nv_udp_t* udp) {
    if (!udp) return -1;
    
    if (udp->socketfd != -1) {
        close(udp->socketfd);
        udp->socketfd = -1;
    }
    
    udp->state = NV_UDP_CLOSED;
    return 0;
}

/* UDP服务器相关函数 */
int nv_udp_server_create(nv_udp_server_t *server, struct nv_loop_s *loop, int port) {
    if (!server || !loop) return -1;
    
    memset(server, 0, sizeof(nv_udp_server_t));
    server->loop = loop;
    server->port = port;
    server->max_clients = 1000;
    
    /* 创建UDP socket */
    server->listener = malloc(sizeof(nv_udp_t));
    if (!server->listener) return -1;
    
    memset(server->listener, 0, sizeof(nv_udp_t));
    server->listener->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->listener->socketfd == -1) {
        perror("NV: UDP socket creation failed");
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
        perror("NV: UDP bind failed");
        close(server->listener->socketfd);
        free(server->listener);
        return -1;
    }
    
    return 0;
}

int nv_udp_server_start(nv_udp_server_t *server) {
    if (!server || !server->listener || server->listener->socketfd == -1) return -1;
    
    server->listener->state = NV_UDP_BOUND;
    
    /* 创建接收数据的事件 */
    nv_event_ext_t *recv_event = malloc(sizeof(nv_event_ext_t));
    if (!recv_event) return -1;
    
    /* 手动初始化事件结构 */
    memset(recv_event, 0, sizeof(nv_event_ext_t));
    recv_event->fd = server->listener->socketfd;
    recv_event->events = NV_EVENT_READ;
    recv_event->data = server;
    recv_event->type = NV_EVENT_TYPE_IO;
    recv_event->priority = NV_EVENT_PRIO_NORMAL;
    
    /* 添加到事件循环 */
    if (nv_loop_add_event(server->loop, recv_event, NV_EVENT_READ) != 0) {
        free(recv_event);
        return -1;
    }
    
    /* 保存事件引用 */
    server->listener->read_event = *recv_event;
    free(recv_event);
    return 0;
}

int nv_udp_server_stop(nv_udp_server_t *server) {
    if (!server) return -1;
    
    if (server->listener && server->listener->socketfd != -1) {
        close(server->listener->socketfd);
        server->listener->socketfd = -1;
        server->listener->state = NV_UDP_CLOSED;
    }
    
    if (server->listener) {
        free(server->listener);
        server->listener = NULL;
    }
    
    return 0;
}

/* UDP客户端相关函数 */
int nv_udp_client_create(nv_udp_client_t *client, struct nv_loop_s *loop) {
    if (!client || !loop) return -1;
    
    memset(client, 0, sizeof(nv_udp_client_t));
    client->loop = loop;
    client->timeout_ms = 5000; /* 默认5秒超时 */
    
    return 0;
}

int nv_udp_client_connect(nv_udp_client_t *client, const char *ip, int port) {
    if (!client || !ip) return -1;
    
    /* 创建UDP socket */
    client->connection = malloc(sizeof(nv_udp_t));
    if (!client->connection) return -1;
    
    memset(client->connection, 0, sizeof(nv_udp_t));
    client->connection->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->connection->socketfd == -1) {
        perror("NV: UDP socket creation failed");
        free(client->connection);
        return -1;
    }
    
    /* 设置目标地址 */
    memset(&client->connection->dest_addr, 0, sizeof(client->connection->dest_addr));
    client->connection->dest_addr.sin_family = AF_INET;
    client->connection->dest_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &client->connection->dest_addr.sin_addr) <= 0) {
        perror("NV: invalid IP address");
        close(client->connection->socketfd);
        free(client->connection);
        return -1;
    }
    
    client->connection->state = NV_UDP_CONNECTED;
    client->server_ip = strdup(ip);
    client->server_port = port;
    return 0;
}

int nv_udp_client_disconnect(nv_udp_client_t *client) {
    if (!client) return -1;
    
    if (client->connection) {
        if (client->connection->socketfd != -1) {
            close(client->connection->socketfd);
            client->connection->socketfd = -1;
        }
        client->connection->state = NV_UDP_CLOSED;
        free(client->connection);
        client->connection = NULL;
    }
    
    if (client->server_ip) {
        free(client->server_ip);
        client->server_ip = NULL;
    }
    
    return 0;
}

/* UDP多播支持 */
int nv_udp_join_multicast(nv_udp_t *udp, const char *multicast_ip, const char *interface_ip) {
    if (!udp || !multicast_ip || udp->socketfd == -1) return -1;
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = interface_ip ? inet_addr(interface_ip) : INADDR_ANY;
    
    return setsockopt(udp->socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

int nv_udp_leave_multicast(nv_udp_t *udp, const char *multicast_ip, const char *interface_ip) {
    if (!udp || !multicast_ip || udp->socketfd == -1) return -1;
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = interface_ip ? inet_addr(interface_ip) : INADDR_ANY;
    
    return setsockopt(udp->socketfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
}

int nv_udp_set_multicast_ttl(nv_udp_t *udp, int ttl) {
    if (!udp || udp->socketfd == -1) return -1;
    
    return setsockopt(udp->socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

int nv_udp_set_multicast_loopback(nv_udp_t *udp, int enable) {
    if (!udp || udp->socketfd == -1) return -1;
    
    return setsockopt(udp->socketfd, IPPROTO_IP, IP_MULTICAST_LOOP, &enable, sizeof(enable));
}

/* UDP广播支持 */
int nv_udp_set_broadcast(nv_udp_t *udp, int enable) {
    if (!udp || udp->socketfd == -1) return -1;
    
    return setsockopt(udp->socketfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
}

int nv_udp_send_broadcast(nv_udp_t *udp, const char *data, size_t len, int port) {
    if (!udp || !data || udp->socketfd == -1) return -1;
    
    /* 设置广播地址 */
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    return sendto(udp->socketfd, data, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
}

/* UDP选项设置函数 */
int nv_udp_set_nonblocking(nv_udp_t *udp) {
    if (!udp || udp->socketfd == -1) return -1;
    
    int flags = fcntl(udp->socketfd, F_GETFL, 0);
    if (flags == -1) return -1;
    
    return fcntl(udp->socketfd, F_SETFL, flags | O_NONBLOCK);
}

int nv_udp_set_reuseaddr(nv_udp_t *udp, int enable) {
    if (!udp || udp->socketfd == -1) return -1;
    
    return setsockopt(udp->socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
}

int nv_udp_set_reuseport(nv_udp_t *udp, int enable) {
    if (!udp || udp->socketfd == -1) return -1;
    
    return setsockopt(udp->socketfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
}

/* UDP缓冲区相关函数 */
int nv_udp_buffer_create(nv_udp_buffer_t *buf, nv_pool_t *pool, size_t initial_size) {
    if (!buf) return -1;
    
    buf->data = malloc(initial_size);
    if (!buf->data) return -1;
    
    buf->size = 0;
    buf->capacity = initial_size;
    buf->pool = pool;
    
    return 0;
}

int nv_udp_buffer_write(nv_udp_buffer_t *buf, const char *data, size_t len) {
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

int nv_udp_buffer_read(nv_udp_buffer_t *buf, char *data, size_t len) {
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

/* UDP连接池相关函数 */
int nv_udp_pool_create(nv_udp_pool_t *pool, struct nv_loop_s *loop, int max_conns) {
    if (!pool) return -1;
    
    pool->connections = malloc(max_conns * sizeof(nv_udp_t*));
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

int nv_udp_pool_return_connection(nv_udp_pool_t *pool, nv_udp_t *conn) {
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