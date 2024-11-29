#ifndef _NV_SOCKET_H_INCLUDED_
#define _NV_SOCKET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_config.h"

//setsockopt 函数用于设置套接字选项，这些选项可以控制套接字的行为。以下是一些常用的套接字选项：
#define  NV_SO_REUSEADDR    SO_REUSEADDR // 允许套接字绑定到一个已经被使用（在TIME_WAIT状态）的本地地址。
#define  NV_SO_REUSEPORT    SO_REUSEPORT //允许多个套接字绑定到同一端口，只要它们使用不同的网络接口。
#define  NV_SO_KEEPALIVE    SO_KEEPALIVE  //启用TCP连接的心跳检测，以保持连接的活跃状态。
#define  NV_SO_LINGER       SO_LINGER    //控制套接字在关闭时的行为，可以设置一个延时以确保所有数据被发送。
#define  NV_SO_BROADCAST    SO_BROADCAST//允许套接字发送广播消息。
//设置套接字发送和接收缓冲区的大小。
#define  NV_SO_SNDBUF  SO_SNDBUF
#define  NV_SO_RCVBUF  SO_RCVBUF       
// 设置发送和接收操作的低水位线。
#define  NV_SO_SNDLOWAT    SO_SNDLOWAT
#define  NV_SO_RCVLOWAT    SO_RCVLOWAT 
//        将带外数据（out-of-band data）直接发送到接收缓冲区，而不是使用recv的带外参数。
#define  NV_SO_OOBINLINE   SO_OOBINLINE
#define  NV_SO_TIMESTAMP   SO_TIMESTAMP//        在接收数据时获取时间戳。

#define  NV_SO_BINDTODEVICE   SO_BINDTODEVICE//将套接字绑定到特定的网络接口。
#define  NV_SO_DEBUG       SO_DEBUG //开启套接字调试信息。
#define  NV_SO_ERROR       SO_ERROR//获取套接字的错误状态。
#define  NV_SO_TYPE        SO_TYPE//获取套接字的类型。
#define  NV_SO_DOMAIN      SO_DOMAIN//获取套接字的协议域。
#define  NV_SO_PROTOCOL    SO_PROTOCOL//获取套接字的协议。
//    获取对端的地址信息。
#define  NV_SO_PEERNAME   SO_PEERNAME
#define  NV_SO_PEERADDR   SO_PEERADDR
#define  NV_SO_ACCEPTCONN  SO_ACCEPTCONN//检查套接字是否已经准备好接受连接。
#define  NV_SO_NO_CHECK    SO_NO_CHECK//禁用UDP数据包的校验和检查。
#define  NV_SO_PRIORITY    SO_PRIORITY//设置套接字的优先级。
#define  NV_SO_MARK        SO_MARK//设置套接字的数据包标记。
#define  NV_TCP_NODELAY    TCP_NODELAY//关闭Nagle算法，即禁用TCP的延迟确认。
#define  NV_TCP_MAXSEG     TCP_MAXSEG//设置TCP的最大报文段大小（MSS）。
//    控制TCP是否合并小的数据包。
#define  NV_TCP_CORK       TCP_CORK 
#define  NV_TCP_NO_CORK     TCP_NO_CORK     
//设置IP数据包的服务类型（TOS）和生存时间（TTL）
#define  NV_IP_TOS         IP_TOS 
#define  NV_IP_TTL         IP_TTL


int nv_tcp_socket_create() ;
int nv_udp_socket_create() ;
int nv_socket_bind(int sockfd, int port) ;
int nv_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) ;
int nv_tcp_socket_listen(int sockfd, int backlog) ;
int nv_tcp_socket_accept(int sockfd) ;
int nv_tcp_socket_connect(int sockfd, const char* ip, int port);
ssize_t nv_socket_send(int sockfd, const void* buffer, size_t length, int flags) ;
ssize_t nv_socket_recv(int sockfd, void* buffer, size_t length, int flags) ;
int nv_socket_close(int sockfd) ;
void run_tcp_server(int port) ;
void run_tcp_client(const char* server_ip, int server_port) ;

ssize_t nv_udp_socket_recvfrom(int sockfd, void *buffer, size_t length, struct sockaddr_in *src_addr) ;
ssize_t nv_udp_socket_sendto(int sockfd, const void *buffer, size_t length, const struct sockaddr_in *dest_addr) ;

int nv_socket_main();
#ifdef __cplusplus
}
#endif

#endif