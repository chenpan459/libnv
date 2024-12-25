/*************
 * 
 * 在用户态实现 ARP 老化功能，并在超时后发送 ARP 请求以刷新 ARP 表，可以通过编写一个程序来定期检查和删除过期的 ARP 表项，并在需要时发送 ARP 请求。
 * 以下是一个示例，展示如
 * 
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>

#define AGING_INTERVAL 60 // 老化间隔（秒）
#define ENTRY_TIMEOUT 300 // 条目超时时间（秒）
#define REPLY_TIMEOUT 5   // 等待回复的超时时间（秒）

struct arp_entry {
    struct in_addr ip;
    struct ether_addr mac;
    time_t timestamp;
};

struct arp_table {
    struct arp_entry *entries;
    size_t size;
    size_t capacity;
};

void init_arp_table(struct arp_table *table, size_t capacity) {
    table->entries = malloc(capacity * sizeof(struct arp_entry));
    table->size = 0;
    table->capacity = capacity;
}

void free_arp_table(struct arp_table *table) {
    free(table->entries);
}

void add_arp_entry(struct arp_table *table, struct in_addr ip, struct ether_addr mac) {
    time_t now = time(NULL);

    // 查找是否已存在
    for (size_t i = 0; i < table->size; i++) {
        if (table->entries[i].ip.s_addr == ip.s_addr) {
            table->entries[i].mac = mac;
            table->entries[i].timestamp = now;
            return;
        }
    }

    // 添加新条目
    if (table->size < table->capacity) {
        table->entries[table->size].ip = ip;
        table->entries[table->size].mac = mac;
        table->entries[table->size].timestamp = now;
        table->size++;
    } else {
        fprintf(stderr, "ARP 表已满\n");
    }
}

void remove_arp_entry(struct arp_table *table, size_t index) {
    if (index < table->size) {
        for (size_t i = index; i < table->size - 1; i++) {
            table->entries[i] = table->entries[i + 1];
        }
        table->size--;
    }
}

int send_arp_request(struct in_addr ip, const char *iface) {
    int sockfd;
    struct sockaddr_ll device;
    struct ifreq ifr;
    unsigned char buffer[42];
    struct ether_header *eth_header = (struct ether_header *) buffer;
    struct ether_arp *arp_header = (struct ether_arp *) (buffer + ETH_HLEN);

    // 创建原始套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("socket 失败");
        return -1;
    }

    // 获取接口的索引
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl 失败");
        close(sockfd);
        return -1;
    }
    device.sll_ifindex = ifr.ifr_ifindex;

    // 获取接口的 MAC 地址
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl 失败");
        close(sockfd);
        return -1;
    }
    memcpy(device.sll_addr, ifr.ifr_hwaddr.sa_data, 6);
    device.sll_halen = ETH_ALEN;

    // 构造以太网头部
    memset(eth_header->ether_dhost, 0xff, 6); // 目标 MAC 地址（广播）
    memcpy(eth_header->ether_shost, device.sll_addr, 6); // 源 MAC 地址
    eth_header->ether_type = htons(ETH_P_ARP);

    // 构造 ARP 头部
    arp_header->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arp_header->ea_hdr.ar_pro = htons(ETH_P_IP);
    arp_header->ea_hdr.ar_hln = ETH_ALEN;
    arp_header->ea_hdr.ar_pln = 4;
    arp_header->ea_hdr.ar_op = htons(ARPOP_REQUEST);
    memcpy(arp_header->arp_sha, device.sll_addr, 6); // 发送方 MAC 地址
    memset(arp_header->arp_tha, 0x00, 6); // 目标 MAC 地址
    memcpy(arp_header->arp_spa, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, 4); // 发送方 IP 地址
    memcpy(arp_header->arp_tpa, &ip, 4); // 目标 IP 地址

    // 发送 ARP 请求
    if (sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&device, sizeof(device)) < 0) {
        perror("sendto 失败");
        close(sockfd);
        return -1;
    }

    // 等待 ARP 回复
    struct timeval timeout = {REPLY_TIMEOUT, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
    if (ret < 0) {
        perror("select 失败");
        close(sockfd);
        return -1;
    } else if (ret == 0) {
        printf("没有收到 ARP 回复\n");
        close(sockfd);
        return -1;
    }

    // 接收 ARP 回复
    unsigned char recv_buffer[42];
    if (recv(sockfd, recv_buffer, sizeof(recv_buffer), 0) < 0) {
        perror("recv 失败");
        close(sockfd);
        return -1;
    }

    struct ether_arp *recv_arp_header = (struct ether_arp *)(recv_buffer + ETH_HLEN);
    if (ntohs(recv_arp_header->ea_hdr.ar_op) == ARPOP_REPLY && memcmp(recv_arp_header->arp_spa, &ip, 4) == 0) {
        printf("收到 ARP 回复\n");
        close(sockfd);
        return 0;
    }

    close(sockfd);
    return -1;
}

void age_arp_entries(struct arp_table *table, const char *iface) {
    time_t now = time(NULL);

    for (size_t i = 0; i < table->size; i++) {
        if (now - table->entries[i].timestamp > ENTRY_TIMEOUT) {
            printf("发送 ARP 请求以验证 ARP 条目: %s\n", inet_ntoa(table->entries[i].ip));
            if (send_arp_request(table->entries[i].ip, iface) != 0) {
                printf("删除过期的 ARP 条目: %s\n", inet_ntoa(table->entries[i].ip));
                remove_arp_entry(table, i);
                i--; // 调整索引以检查移动后的条目
            } else {
                table->entries[i].timestamp = now; // 更新时间戳
            }
        }
    }
}

void print_arp_table(struct arp_table *table) {
    printf("ARP 表:\n");
    for (size_t i = 0; i < table->size; i++) {
        printf("%s -> %s\n", inet_ntoa(table->entries[i].ip), ether_ntoa(&table->entries[i].mac));
    }
}

int main() {
    struct arp_table table;
    init_arp_table(&table, 1024);

    struct in_addr ip1, ip2;
    struct ether_addr mac1, mac2;

    inet_aton("192.168.1.1", &ip1);
    inet_aton("192.168.1.2", &ip2);
    ether_aton_r("00:11:22:33:44:55", &mac1);
    ether_aton_r("66:77:88:99:aa:bb", &mac2);

    add_arp_entry(&table, ip1, mac1);
    add_arp_entry(&table, ip2, mac2);

    const char *iface = "eth0";

    while (1) {
        print_arp_table(&table);
        age_arp_entries(&table, iface);
        sleep(AGING_INTERVAL);
    }

    free_arp_table(&table);
    return 0;
}