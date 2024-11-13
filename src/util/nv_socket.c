
#include "nv_socket.h"



// 创建 TCP 套接字
int nv_tcp_socket_create() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("nv_tcp_socket_create 失败");
    }
    return sockfd;
}

// 创建 UDP 套接字
int nv_udp_socket_create() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("nv_udp_socket_create 失败");
    }
    return sockfd;
}

// 绑定套接字到地址和端口
int nv_socket_bind(int sockfd, const char* ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int bind_result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (bind_result < 0) {
        perror("nv_socket_bind 失败");
    }
    return bind_result;
}

// 监听连接
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
    ssize_t send_result = send(sockfd, buffer, length, flags);
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

// 示例：简单的 TCP 服务器
void run_tcp_server(const char* ip, int port) {
    int server_fd = nv_tcp_socket_create();
    if (server_fd < 0) exit(EXIT_FAILURE);

    if (nv_socket_bind(server_fd, ip, port) < 0) exit(EXIT_FAILURE);
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


