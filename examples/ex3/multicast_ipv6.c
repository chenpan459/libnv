#include <stdio.h>
#include <nv_socket.h>

int main() {
   // ignore_sigpipe();

    // 创建 UDP 套接字
    int udp_sockfd = nv_udp_socket_create_ipv6();
    if (udp_sockfd < 0) {
        return 1;
    }

    // 绑定 UDP 套接字到端口 9090
    if (nv_socket_bind_ipv6(udp_sockfd, 9090) < 0) {
        return 1;
    }

    // 加入组播组
    const char *multicast_ip = "ff02::1";
    const char *interface_ip = "eth0";
    if (nv_join_multicast_group_ipv6(udp_sockfd, multicast_ip, interface_ip) < 0) {
        return 1;
    }

    // 设置组播 TTL
    if (nv_set_multicast_ttl_ipv6(udp_sockfd, 1) < 0) {
        return 1;
    }

    // 设置组播接口
    if (nv_set_multicast_interface_ipv6(udp_sockfd, interface_ip) < 0) {
        return 1;
    }

    // 发送组播数据
    // 发送组播数据
    struct sockaddr_in6 dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(9090);
    inet_pton(AF_INET6, multicast_ip, &dest_addr.sin6_addr);

    const char *message = "Hello, Multicast!";
    nv_udp_socket_sendto_ipv6(udp_sockfd, message, strlen(message), &dest_addr);

    // 接收组播数据
    char buffer[1024];
    struct sockaddr_in src_addr;
    ssize_t bytes_received = nv_udp_socket_recvfrom_ipv6(udp_sockfd, buffer, sizeof(buffer), &src_addr);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("接收到: %s\n", buffer);
    }

    // 离开组播组
    if (nv_leave_multicast_group_ipv6(udp_sockfd, multicast_ip, interface_ip) < 0) {
        return 1;
    }

    // 关闭套接字
    nv_socket_close(udp_sockfd);

    return 0;
}