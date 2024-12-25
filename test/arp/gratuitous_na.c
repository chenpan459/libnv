#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <netpacket/packet.h>

#define BUFFER_SIZE 128

/* IPv6 的 Gratuitous ARP 等效操作是发送邻居通告（Neighbor Advertisement，NA）消息

   使用方法:
   sudo ./gratuitous_na eth0 fe80::1
*/
int main(int argc, char *argv[]) {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_ll device;
    unsigned char buffer[BUFFER_SIZE];
    struct ether_header *eth_header = (struct ether_header *) buffer;
    struct ip6_hdr *ip6_header = (struct ip6_hdr *) (buffer + ETH_HLEN);
    struct nd_neighbor_advert *na_header = (struct nd_neighbor_advert *) (buffer + ETH_HLEN + sizeof(struct ip6_hdr));
    struct nd_opt_hdr *opt_header = (struct nd_opt_hdr *) (buffer + ETH_HLEN + sizeof(struct ip6_hdr) + sizeof(struct nd_neighbor_advert));
    unsigned char src_mac[6];
    char *iface, *src_ip;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <source_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    iface = argv[1];
    src_ip = argv[2];

    // 创建原始套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
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
    eth_header->ether_type = htons(ETH_P_IPV6);

    // 构造 IPv6 头部
    ip6_header->ip6_flow = htonl((6 << 28) | (0 << 20) | 0); // 版本、流量类别、流标签
    ip6_header->ip6_plen = htons(sizeof(struct nd_neighbor_advert) + sizeof(struct nd_opt_hdr) + 6); // 负载长度
    ip6_header->ip6_nxt = IPPROTO_ICMPV6; // 下一头部
    ip6_header->ip6_hlim = 255; // 跳数限制
    inet_pton(AF_INET6, src_ip, &ip6_header->ip6_src); // 源 IPv6 地址
    inet_pton(AF_INET6, "ff02::1", &ip6_header->ip6_dst); // 目标 IPv6 地址（全节点多播地址）

    // 构造邻居通告头部
    na_header->nd_na_type = ND_NEIGHBOR_ADVERT;
    na_header->nd_na_code = 0;
    na_header->nd_na_cksum = 0; // 校验和稍后计算
    na_header->nd_na_flags_reserved = htonl(ND_NA_FLAG_OVERRIDE);
    inet_pton(AF_INET6, src_ip, &na_header->nd_na_target); // 目标地址（与源地址相同）

    // 构造邻居通告选项头部
    opt_header->nd_opt_type = ND_OPT_TARGET_LINKADDR;
    opt_header->nd_opt_len = 1;
    memcpy(opt_header + 1, src_mac, 6); // 目标链路层地址

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

    // 计算 ICMPv6 校验和
    struct {
        struct in6_addr src;
        struct in6_addr dst;
        uint32_t len;
        uint8_t zero[3];
        uint8_t nxt;
    } pseudo_header;
    memset(&pseudo_header, 0, sizeof(pseudo_header));
    pseudo_header.src = ip6_header->ip6_src;
    pseudo_header.dst = ip6_header->ip6_dst;
    pseudo_header.len = ip6_header->ip6_plen;
    pseudo_header.nxt = ip6_header->ip6_nxt;

    unsigned char cksum_buffer[BUFFER_SIZE + sizeof(pseudo_header)];
    memcpy(cksum_buffer, &pseudo_header, sizeof(pseudo_header));
    memcpy(cksum_buffer + sizeof(pseudo_header), na_header, ntohs(ip6_header->ip6_plen));
    na_header->nd_na_cksum = csum_ipv6_magic(&ip6_header->ip6_src, &ip6_header->ip6_dst, ntohs(ip6_header->ip6_plen), IPPROTO_ICMPV6, csum_partial(cksum_buffer, sizeof(cksum_buffer), 0));

    // 发送邻居通告消息
    if (sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &device, sizeof(device)) < 0) {
        perror("sendto 失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("邻居通告消息已发送\n");

    close(sockfd);
    return 0;
}