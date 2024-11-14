#ifndef _NV_SOCKET_H_INCLUDED_
#define _NV_SOCKET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_util_include.h"

int nv_tcp_socket_create() ;
int nv_udp_socket_create() ;
int nv_socket_bind(int sockfd, const char* ip, int port) ;
int nv_tcp_socket_listen(int sockfd, int backlog) ;
int nv_tcp_socket_accept(int sockfd) ;
int nv_tcp_socket_connect(int sockfd, const char* ip, int port);
ssize_t nv_socket_send(int sockfd, const void* buffer, size_t length, int flags) ;
ssize_t nv_socket_recv(int sockfd, void* buffer, size_t length, int flags) ;
int nv_socket_close(int sockfd) ;
void run_tcp_server(const char* ip, int port) ;
void run_tcp_client(const char* server_ip, int server_port) ;


int nv_socket_main();
#ifdef __cplusplus
}
#endif

#endif