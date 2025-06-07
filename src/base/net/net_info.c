#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>   // struct ifconf 和 ioctl 定义
#include <net/if.h>      // struct ifreq, ifconf 等网络接口结构
#include <unistd.h>



int get_net_linkUp_info() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifconf ifc;
    char buf[1024*10];
    struct ifreq *ifr;

    if (sock == -1) {
        perror("socket");
        return 1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;

    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        perror("ioctl(SIOCGIFCONF)");
        close(sock);
        return 1;
    }
    ifr = ifc.ifc_req;
    int num_interfaces = ifc.ifc_len / sizeof(struct ifreq);

    printf("num Interface: %d\n", num_interfaces);


    for (int i = 0; i < ifc.ifc_len / sizeof(struct ifreq); ++i) {
        ifr = &((struct ifreq*)buf)[i];

            printf("Interface: i=%d\n", i);
        // 获取 IP 地址
        struct sockaddr_in *sin = (struct sockaddr_in *)&ifr->ifr_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin->sin_addr, ip, INET_ADDRSTRLEN);

        // 获取子网掩码
        if (ioctl(sock, SIOCGIFNETMASK, ifr) == 0) {
            sin = (struct sockaddr_in *)&ifr->ifr_netmask;
            char mask[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sin->sin_addr, mask, INET_ADDRSTRLEN);

            printf("Interface: %s\n", ifr->ifr_name);
            printf("  IP Address: %s\n", ip);
            printf("  Subnet Mask: %s\n\n", mask);
        }else{
            printf("Interface: %s\n", ifr->ifr_name);
            printf("  IP Address: NULL\n");
            printf("  Subnet Mask: NULL\n\n");

	    }
    }

    close(sock);
    return 0;
}

int get_net_info() {
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET6_ADDRSTRLEN];
    char mask[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        printf("Interface: %s\n", ifa->ifa_name);

        // IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;

            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            inet_ntop(AF_INET, &netmask->sin_addr, mask, sizeof(mask));

            printf("  IPv4 Address : %s\n", ip);
            printf("  Netmask      : %s\n", mask);
        }
        // IPv6
        else if (ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
            struct sockaddr_in6 *netmask = (struct sockaddr_in6 *)ifa->ifa_netmask;

            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            inet_ntop(AF_INET6, &netmask->sin6_addr, mask, sizeof(mask));

            printf("  IPv6 Address : %s\n", ip);
            printf("  Netmask      : %s\n", mask);
        }
        printf("\n");
    }

    freeifaddrs(ifaddr);
    return 0;
}
