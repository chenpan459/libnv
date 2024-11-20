#include "nv_unix_socket.h"

/*为了封装 Unix 套接字通信接口，可以创建一个包含创建、绑定、连接、发送和接收数据等功能的接口。展示如何封装 Unix 套接字通信接口。*/

// 创建 Unix 套接字
int nv_unix_socket_create() {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("nv_unix_socket_create 失败");
    }
    return sockfd;
}

// 绑定 Unix 套接字到路径
int nv_unix_socket_bind(int sockfd, const char* path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    unlink(path); // 确保路径不存在

    int bind_result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (bind_result < 0) {
        perror("nv_unix_socket_bind 失败");
    }
    return bind_result;
}

// 连接到 Unix 套接字
int nv_unix_socket_connect(int sockfd, const char* path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    int connect_result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (connect_result < 0) {
        perror("nv_unix_socket_connect 失败");
    }
    return connect_result;
}

// 发送数据到 Unix 套接字
ssize_t nv_unix_socket_send(int sockfd, const void* buffer, size_t length) {
    ssize_t sent_length = send(sockfd, buffer, length, 0);
    if (sent_length < 0) {
        perror("nv_unix_socket_send 失败");
    }
    return sent_length;
}

// 接收来自 Unix 套接字的数据
ssize_t nv_unix_socket_receive(int sockfd, void* buffer, size_t length) {
    ssize_t received_length = recv(sockfd, buffer, length, 0);
    if (received_length < 0) {
        perror("nv_unix_socket_receive 失败");
    }
    return received_length;
}

// 关闭 Unix 套接字
int nv_unix_socket_close(int sockfd) {
    int close_result = close(sockfd);
    if (close_result < 0) {
        perror("nv_unix_socket_close 失败");
    }
    return close_result;
}

// 设置发送缓冲区大小
int nv_unix_socket_set_send_buffer_size(int sockfd, int size) {
    int result = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
    if (result < 0) {
        perror("nv_unix_socket_set_send_buffer_size 失败");
    }
    return result;
}

// 设置接收缓冲区大小
int nv_unix_socket_set_receive_buffer_size(int sockfd, int size) {
    int result = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if (result < 0) {
        perror("nv_unix_socket_set_receive_buffer_size 失败");
    }
    return result;
}



#if NV_UTIL_TEST_ON

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUFFER_SIZE 256
#define SEND_BUFFER_SIZE 8192
#define RECEIVE_BUFFER_SIZE 8192

int nv_unix_socket_server_main() {
    int server_fd, client_fd;
    char buffer[BUFFER_SIZE];

    // 创建和绑定 Unix 套接字
    server_fd = nv_unix_socket_create();
    if (server_fd < 0) {
        return 1;
    }

    if (nv_unix_socket_bind(server_fd, SOCKET_PATH) < 0) {
        nv_unix_socket_close(server_fd);
        return 1;
    }

    // 设置缓冲区大小
    nv_unix_socket_set_send_buffer_size(server_fd, SEND_BUFFER_SIZE);
    nv_unix_socket_set_receive_buffer_size(server_fd, RECEIVE_BUFFER_SIZE);

    // 监听连接
    if (listen(server_fd, 5) < 0) {
        perror("listen 失败");
        nv_unix_socket_close(server_fd);
        return 1;
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // 接受连接
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept 失败");
        nv_unix_socket_close(server_fd);
        return 1;
    }

    // 接收数据
    memset(buffer, 0, BUFFER_SIZE);
    if (nv_unix_socket_receive(client_fd, buffer, BUFFER_SIZE) < 0) {
        nv_unix_socket_close(client_fd);
        nv_unix_socket_close(server_fd);
        return 1;
    }

    printf("Received message: %s\n", buffer);

    // 发送响应
    const char *response = "Hello from server";
    if (nv_unix_socket_send(client_fd, response, strlen(response)) < 0) {
        nv_unix_socket_close(client_fd);
        nv_unix_socket_close(server_fd);
        return 1;
    }

    // 关闭连接
    nv_unix_socket_close(client_fd);
    nv_unix_socket_close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}




int nv_unix_socket_client_main() {
    int client_fd;
    char buffer[BUFFER_SIZE];

    // 创建 Unix 套接字
    client_fd = nv_unix_socket_create();
    if (client_fd < 0) {
        return 1;
    }

    // 连接到服务器
    if (nv_unix_socket_connect(client_fd, SOCKET_PATH) < 0) {
        nv_unix_socket_close(client_fd);
        return 1;
    }

    // 设置缓冲区大小
    nv_unix_socket_set_send_buffer_size(client_fd, SEND_BUFFER_SIZE);
    nv_unix_socket_set_receive_buffer_size(client_fd, RECEIVE_BUFFER_SIZE);

    // 发送数据
    const char *message = "Hello from client";
    if (nv_unix_socket_send(client_fd, message, strlen(message)) < 0) {
        nv_unix_socket_close(client_fd);
        return 1;
    }

    // 接收响应
    memset(buffer, 0, BUFFER_SIZE);
    if (nv_unix_socket_receive(client_fd, buffer, BUFFER_SIZE) < 0) {
        nv_unix_socket_close(client_fd);
        return 1;
    }

    printf("Received response: %s\n", buffer);

    // 关闭连接
    nv_unix_socket_close(client_fd);

    return 0;
}

#endif 