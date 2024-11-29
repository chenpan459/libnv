#include <stdio.h>
#include <nv_socket.h>

int main() {
   // ignore_sigpipe();

    // 创建 UDP 套接字
    int udp_sockfd = nv_udp_socket_create();
    if (udp_sockfd < 0) {
        return 1;
    }

    // 绑定 UDP 套接字到端口 9090
    if (nv_socket_bind(udp_sockfd, 9090) < 0) {
        return 1;
    }

    // 加入组播组
    const char *multicast_ip = "239.0.0.1";
    const char *interface_ip = "0.0.0.0";
    if (nv_join_multicast_group(udp_sockfd, multicast_ip, interface_ip) < 0) {
        return 1;
    }

    // 设置组播 TTL
    if (nv_set_multicast_ttl(udp_sockfd, 1) < 0) {
        return 1;
    }

    // 发送组播数据
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(9090);
    inet_pton(AF_INET, multicast_ip, &dest_addr.sin_addr);

    const char *message = "Hello, Multicast!";
    nv_udp_socket_sendto(udp_sockfd, message, strlen(message), &dest_addr);

    // 接收组播数据
    char buffer[1024];
    struct sockaddr_in src_addr;
    ssize_t bytes_received = nv_udp_socket_recvfrom(udp_sockfd, buffer, sizeof(buffer), &src_addr);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("接收到: %s\n", buffer);
    }

    // 离开组播组
    if (nv_leave_multicast_group(udp_sockfd, multicast_ip, interface_ip) < 0) {
        return 1;
    }

    // 关闭套接字
    nv_socket_close(udp_sockfd);

    return 0;
}