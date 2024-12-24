#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>

#define BUFFER_SIZE 42

/*Gratuitous ARP 是一种特殊的 ARP 请求报文，用于通知网络中的其他设备本机的 MAC 地址变化

sudo ./arp_broadcast eth0 192.168.1.100
*/
int main(int argc, char *argv[]) {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_ll device;
    unsigned char buffer[BUFFER_SIZE];
    struct ether_header *eth_header = (struct ether_header *) buffer;
    struct ether_arp *arp_header = (struct ether_arp *) (buffer + ETH_HLEN);
    unsigned char src_mac[6];
    char *iface, *src_ip;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <source_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    iface = argv[1];
    src_ip = argv[2];

    // 创建原始套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("socket 失败");
        exit(EXIT_FAILURE);
    }

    // 获取接口的 MAC 地址
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl 失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6);

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
    inet_pton(AF_INET, src_ip, arp_header->arp_tpa); // 目标 IP 地址（与发送方 IP 地址相同）

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

    // 发送 Gratuitous ARP 报文
    if (sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &device, sizeof(device)) < 0) {
        perror("sendto 失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Gratuitous ARP 报文已发送\n");

    close(sockfd);
    return 0;
}