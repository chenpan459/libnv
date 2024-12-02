#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void process_icmpv6_packet(const struct icmp6_hdr *icmp_hdr) {
    switch (icmp_hdr->icmp6_type) {
        case ICMP6_ECHO_REQUEST:
            printf("ICMPv6 Echo Request\n");
            break;
        case ICMP6_ECHO_REPLY:
            printf("ICMPv6 Echo Reply\n");
            break;
        default:
            printf("ICMPv6 Type: %d\n", icmp_hdr->icmp6_type);
            break;
    }
}

int main() {
    int sockfd;
    struct sockaddr_in6 addr;
    char buffer[BUFFER_SIZE];

    // 创建原始套接字
    sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (sockfd < 0) {
        perror("socket 失败");
        return 1;
    }

    // 绑定到所有接口
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind 失败");
        close(sockfd);
        return 1;
    }

    printf("开始监听 ICMPv6 报文...\n");

    while (1) {
        struct sockaddr_in6 src_addr;
        socklen_t addr_len = sizeof(src_addr);
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&src_addr, &addr_len);
        if (bytes_received < 0) {
            perror("recvfrom 失败");
            continue;
        }

        struct ip6_hdr *ip6_hdr = (struct ip6_hdr *)buffer;
        struct icmp6_hdr *icmp_hdr = (struct icmp6_hdr *)(buffer + sizeof(struct ip6_hdr));

        char src_ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &src_addr.sin6_addr, src_ip, sizeof(src_ip));
        printf("接收到来自 %s 的 ICMPv6 报文\n", src_ip);

        process_icmpv6_packet(icmp_hdr);
    }

    close(sockfd);
    return 0;
}