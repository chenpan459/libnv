#include "nv_unix_socket.h"

/*为了封装 Unix 套接字通信接口，可以创建一个包含创建、绑定、连接、发送和接收数据等功能的接口。展示如何封装 Unix 套接字通信接口。
nv_unix_socket_create：创建 Unix 套接字。
nv_unix_socket_bind：绑定 Unix 套接字到指定路径。
nv_unix_socket_connect：连接到指定路径的 Unix 套接字。
nv_unix_socket_send：发送数据到 Unix 套接字。
nv_unix_socket_receive：接收来自 Unix 套接字的数据。
nv_unix_socket_close：关闭 Unix 套接字。
*/

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




#endif
