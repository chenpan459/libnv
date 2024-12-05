#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>

#define INTERFACE "ens37"
#define SRC_IP "192.168.1.100"
#define DST_IP "192.168.1.1"

void get_mac_address(const char *iface, unsigned char *mac) {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
}

void send_arp_broadcast(const char *iface, const char *src_ip, const char *dst_ip) {
    int sockfd;
    unsigned char buffer[42];
    struct ether_header *eth_header = (struct ether_header *)buffer;
    struct ether_arp *arp_header = (struct ether_arp *)(buffer + sizeof(struct ether_header));
    struct sockaddr_ll device;
    unsigned char src_mac[6];

    // 获取源 MAC 地址
    get_mac_address(iface, src_mac);

    // 创建原始套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("socket 失败");
        exit(EXIT_FAILURE);
    }

    // 构造以太网头部
    memset(eth_header->ether_dhost, 0xff, 6); // 目标 MAC 地址（广播）
    memcpy(eth_header->ether_shost, src_mac, 6); // 源 MAC 地址
    eth_header->ether_type = htons(ETHERTYPE_ARP);

    // 构造 ARP 头部
    arp_header->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arp_header->ea_hdr.ar_pro = htons(ETHERTYPE_IP);
    arp_header->ea_hdr.ar_hln = 6;
    arp_header->ea_hdr.ar_pln = 4;
    arp_header->ea_hdr.ar_op = htons(ARPOP_REQUEST);
    memcpy(arp_header->arp_sha, src_mac, 6); // 发送方 MAC 地址
    inet_pton(AF_INET, src_ip, arp_header->arp_spa); // 发送方 IP 地址
    memset(arp_header->arp_tha, 0x00, 6); // 目标 MAC 地址
    inet_pton(AF_INET, dst_ip, arp_header->arp_tpa); // 目标 IP 地址

    // 设置设备信息
    memset(&device, 0, sizeof(device));
    if ((device.sll_ifindex = if_nametoindex(iface)) == 0) {
        perror("if_nametoindex 失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    device.sll_family = AF_PACKET;
    memcpy(device.sll_addr, src_mac, 6);
    device.sll_halen = htons(6);

    // 发送 ARP 广播报文
    if (sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&device, sizeof(device)) <= 0) {
        perror("sendto 失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("ARP 广播报文已发送\n");

    close(sockfd);
}

int main() {
    send_arp_broadcast(INTERFACE, SRC_IP, DST_IP);
    return 0;
}