
#include "nv_socket.h"



// 创建 TCP 套接字
int nv_tcp_socket_create() {
    int reuse = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("nv_tcp_socket_create 失败");
    }
  
   //设置SO_REUSEADDR选项，以允许立即重新使用地址和端口。
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
   #if 0 
 // 设置 SO_REUSEADDR 选项
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    // 设置 SO_KEEPALIVE 选项,启用 TCP 保活机制
    int keepalive = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    // 设置接收缓冲区大小
    int rcvbuf_size = 1024 * 1024; // 1 MB
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size));
    // 设置发送缓冲区大小
    int sndbuf_size = 1024 * 1024; // 1 MB
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, sizeof(sndbuf_size));
    // 设置延迟关闭选项
    struct linger linger_opt = {1, 5}; // 启用延迟关闭，延迟时间为 5 秒
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
    // 禁用 Nagle 算法
    int nodelay = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    // 设置 IP 数据包的生存时间（TTL）
    int ttl = 64;
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
#endif
    return sockfd;
}


// 创建 UDP 套接字
int nv_udp_socket_create() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("nv_udp_socket_create 失败");
    }
    int broadcast = 1;
    nv_setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    return sockfd;
}

// 绑定套接字到地址和端口
int nv_socket_bind(int sockfd,int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
   // inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_addr.s_addr = INADDR_ANY; // 绑定到系统的任意 IP 地址


    int bind_result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (bind_result < 0) {
        perror("nv_socket_bind 失败");
    }
    return bind_result;
}


// 设置套接字选项
int nv_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    int result = setsockopt(sockfd, level, optname, optval, optlen);
    if (result < 0) {
        perror("nv_setsockopt 失败");
    }
    return result;
}

/********************************************************
 * 监听连接
 * sockfd：这是一个指向 socket 文件描述符的整型变量，该文件描述符必须指向一个已经创建且未被绑定的 socket。
 * backlog：这是一个整型参数，表示内核应该为相应 socket 维护的未完成连接的最大数量。这个参数指定了内核应该为该 socket 排队的最大连接个数。
 * ************************************************************/
int nv_tcp_socket_listen(int sockfd, int backlog) {
    int listen_result = listen(sockfd, backlog);
    if (listen_result < 0) {
        perror("nv_tcp_socket_listen 失败");
    }
    return listen_result;
}

// 接受连接
int nv_tcp_socket_accept(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (client_sockfd < 0) {
        perror("nv_tcp_socket_accept 失败");
    }
    return client_sockfd;
}

// 连接到服务器
int nv_tcp_socket_connect(int sockfd, const char* ip, int port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    int connect_result = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connect_result < 0) {
        perror("nv_tcp_socket_connect 失败");
    }
    return connect_result;
}


// 发送数据
ssize_t nv_socket_send(int sockfd, const void* buffer, size_t length, int flags) {
    ssize_t send_result = send(sockfd, buffer, length, flags | MSG_NOSIGNAL);
    if (send_result < 0) {
        perror("nv_socket_send 失败");
    }
    return send_result;
}

// 接收数据
ssize_t nv_socket_recv(int sockfd, void* buffer, size_t length, int flags) {
    ssize_t recv_result = recv(sockfd, buffer, length, flags);
    if (recv_result < 0) {
        perror("nv_socket_recv 失败");
    }
    return recv_result;
}

// 关闭套接字
int nv_socket_close(int sockfd) {
    return close(sockfd);
}

// 发送 UDP 数据
ssize_t nv_udp_socket_sendto(int sockfd, const void *buffer, size_t length, const struct sockaddr_in *dest_addr) {
    ssize_t send_result = sendto(sockfd, buffer, length, 0, (const struct sockaddr *)dest_addr, sizeof(*dest_addr));
    if (send_result < 0) {
        perror("nv_udp_sendto 失败");
    }
    return send_result;
}

// 接收 UDP 数据
ssize_t nv_udp_socket_recvfrom(int sockfd, void *buffer, size_t length, struct sockaddr_in *src_addr) {
    socklen_t addr_len = sizeof(*src_addr);
    ssize_t recv_result = recvfrom(sockfd, buffer, length, 0, (struct sockaddr *)src_addr, &addr_len);
    if (recv_result < 0) {
        perror("nv_udp_recvfrom 失败");
    }
    return recv_result;
}




// 加入组播组
int nv_join_multicast_group(int sockfd, const char *multicast_ip, const char *interface_ip) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = inet_addr(interface_ip);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("nv_join_multicast_group 失败");
        return -1;
    }
    return 0;
}

// 离开组播组
int nv_leave_multicast_group(int sockfd, const char *multicast_ip, const char *interface_ip) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = inet_addr(interface_ip);
    if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("nv_leave_multicast_group 失败");
        return -1;
    }
    return 0;
}

// 设置组播 TTL
int nv_set_multicast_ttl(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("nv_set_multicast_ttl 失败");
        return -1;
    }
    return 0;
}

// 设置组播接口
int nv_set_multicast_interface(int sockfd, const char *interface_ip) {
    struct in_addr local_interface;
    local_interface.s_addr = inet_addr(interface_ip);
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &local_interface, sizeof(local_interface)) < 0) {
        perror("nv_set_multicast_interface 失败");
        return -1;
    }
    return 0;
}

// 创建 IPv6 TCP 套接字
int nv_tcp_socket_create_ipv6(void) {
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("nv_tcp_socket_create_ipv6 失败");
    }

    // 设置常用的套接字选项
    int reuse = 1;
    nv_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int keepalive = 1;
    nv_setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

    int rcvbuf_size = 1024 * 1024; // 1 MB
    nv_setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size));

    int sndbuf_size = 1024 * 1024; // 1 MB
    nv_setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, sizeof(sndbuf_size));

    struct linger linger_opt = {1, 5}; // 启用延迟关闭，延迟时间为 5 秒
    nv_setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

    int nodelay = 1;
    nv_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    int ttl = 64;
    nv_setsockopt(sockfd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));

    return sockfd;
}
// 绑定 IPv6 套接字到地址和端口
int nv_socket_bind_ipv6(int sockfd, int port) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any; // 绑定到系统的任意 IPv6 地址

    int bind_result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_result < 0) {
        perror("nv_socket_bind_ipv6 失败");
    }
    return bind_result;
}

// 连接到 IPv6 服务器
int nv_tcp_socket_connect_ipv6(int sockfd, const char *ip, int port) {
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
        perror("inet_pton 失败");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("nv_tcp_socket_connect_ipv6 失败");
        return -1;
    }
    return 0;
}


// 创建 IPv6 UDP 套接字
int nv_udp_socket_create_ipv6(void) {
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("nv_udp_socket_create_ipv6 失败");
    }

    int broadcast = 1;
    nv_setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    return sockfd;
}

// 发送 IPv6 UDP 数据
ssize_t nv_udp_socket_sendto_ipv6(int sockfd, const void *buffer, size_t length, const struct sockaddr_in6 *dest_addr) {
    ssize_t send_result = sendto(sockfd, buffer, length, 0, (const struct sockaddr *)dest_addr, sizeof(*dest_addr));
    if (send_result < 0) {
        perror("nv_udp_sendto_ipv6 失败");
    }
    return send_result;
}
// 接收 IPv6 UDP 数据
ssize_t nv_udp_socket_recvfrom_ipv6(int sockfd, void *buffer, size_t length, struct sockaddr_in6 *src_addr) {
    socklen_t addr_len = sizeof(*src_addr);
    ssize_t recv_result = recvfrom(sockfd, buffer, length, 0, (struct sockaddr *)src_addr, &addr_len);
    if (recv_result < 0) {
        perror("nv_udp_recvfrom_ipv6 失败");
    }
    return recv_result;
}

// 加入 IPv6 组播组
int nv_join_multicast_group_ipv6(int sockfd, const char *multicast_ip, const char *interface_ip) {
    struct ipv6_mreq mreq;
    if (inet_pton(AF_INET6, multicast_ip, &mreq.ipv6mr_multiaddr) <= 0) {
        perror("inet_pton 失败");
        return -1;
    }
    mreq.ipv6mr_interface = if_nametoindex(interface_ip);
    if (mreq.ipv6mr_interface == 0) {
        perror("if_nametoindex 失败");
        return -1;
    }
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0) {
        perror("nv_join_multicast_group_ipv6 失败");
        return -1;
    }
    return 0;
}

// 离开 IPv6 组播组
int nv_leave_multicast_group_ipv6(int sockfd, const char *multicast_ip, const char *interface_ip) {
    struct ipv6_mreq mreq;
    if (inet_pton(AF_INET6, multicast_ip, &mreq.ipv6mr_multiaddr) <= 0) {
        perror("inet_pton 失败");
        return -1;
    }
    mreq.ipv6mr_interface = if_nametoindex(interface_ip);
    if (mreq.ipv6mr_interface == 0) {
        perror("if_nametoindex 失败");
        return -1;
    }
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq, sizeof(mreq)) < 0) {
        perror("nv_leave_multicast_group_ipv6 失败");
        return -1;
    }
    return 0;
}

// 设置 IPv6 组播 TTL
int nv_set_multicast_ttl_ipv6(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        perror("nv_set_multicast_ttl_ipv6 失败");
        return -1;
    }
    return 0;
}

// 设置 IPv6 组播接口
int nv_set_multicast_interface_ipv6(int sockfd, const char *interface_ip) {
    unsigned int ifindex = if_nametoindex(interface_ip);
    if (ifindex == 0) {
        perror("if_nametoindex 失败");
        return -1;
    }
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) < 0) {
        perror("nv_set_multicast_interface_ipv6 失败");
        return -1;
    }
    return 0;
}




// 示例：简单的 TCP 服务器
void run_tcp_server(int port) {
    int server_fd = nv_tcp_socket_create();
    if (server_fd < 0) exit(EXIT_FAILURE);

    if (nv_socket_bind(server_fd, port) < 0) exit(EXIT_FAILURE);
    if (nv_tcp_socket_listen(server_fd, 5) < 0) exit(EXIT_FAILURE);

    printf("等待客户端连接...\n");
    int client_fd = nv_tcp_socket_accept(server_fd);
    if (client_fd < 0) exit(EXIT_FAILURE);

    char buffer[1024];
    ssize_t bytes_received;
    while ((bytes_received = nv_socket_recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("接收到: %s\n", buffer);
        nv_socket_send(client_fd, buffer, bytes_received, 0);
    }

    nv_socket_close(client_fd);
    nv_socket_close(server_fd);
}

// 示例：简单的 TCP 客户端
void run_tcp_client(const char* server_ip, int server_port) {
    int client_fd = nv_tcp_socket_create();
    if (client_fd < 0) exit(EXIT_FAILURE);

    if (nv_tcp_socket_connect(client_fd, server_ip, server_port) < 0) exit(EXIT_FAILURE);

    const char* message = "Hello, Server!";
    nv_socket_send(client_fd, message, strlen(message), 0);

    char buffer[1024];
    ssize_t bytes_received = nv_socket_recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("接收到: %s\n", buffer);
    }

    nv_socket_close(client_fd);
}

// 示例：简单的 UDP 服务器
int udp_server_main() {

    // 创建 UDP 套接字
    int udp_sockfd = nv_udp_socket_create();
    if (udp_sockfd < 0) {
        return 1;
    }

    // 绑定 UDP 套接字到端口 9090
    if (nv_socket_bind(udp_sockfd, 9090) < 0) {
        return 1;
    }

    // 接收数据
    char buffer[1024];
    struct sockaddr_in src_addr;
    while (1) {
        ssize_t bytes_received = nv_udp_socket_recvfrom(udp_sockfd, buffer, sizeof(buffer), &src_addr);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("接收到: %s\n", buffer);

            // 发送响应
            const char *response = "Hello, UDP Client!";
            nv_udp_socket_sendto(udp_sockfd, response, strlen(response), &src_addr);
        }
    }

    // 关闭套接字
    nv_socket_close(udp_sockfd);

    return 0;
}

int nv_socket_main()
{

   // 运行 TCP 服务器（在单独的终端中执行）
    // run_tcp_server("127.0.0.1", 8080);

    // 运行 TCP 客户端
    run_tcp_client("127.0.0.1", 8080);


    return 0;
}
