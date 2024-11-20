#ifndef _NV_NUIX_SOCKET_H_INCLUDED_
#define _NV_NUIX_SOCKET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


// 创建 Unix 套接字
int nv_unix_socket_create();

// 绑定 Unix 套接字到路径
int nv_unix_socket_bind(int sockfd, const char* path);

// 连接到 Unix 套接字
int nv_unix_socket_connect(int sockfd, const char* path);

// 发送数据到 Unix 套接字
ssize_t nv_unix_socket_send(int sockfd, const void* buffer, size_t length);

// 接收来自 Unix 套接字的数据
ssize_t nv_unix_socket_receive(int sockfd, void* buffer, size_t length);

// 关闭 Unix 套接字
int nv_unix_socket_close(int sockfd);

// 设置发送缓冲区大小
int nv_unix_socket_set_send_buffer_size(int sockfd, int size);

// 设置接收缓冲区大小
int nv_unix_socket_set_receive_buffer_size(int sockfd, int size);

int  nv_unix_socket_main();
#ifdef __cplusplus
}
#endif

#endif