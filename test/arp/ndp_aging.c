/****
 * 
 * 
 * 
 * ping6 命令来发送 ICMPv6 邻居请求（Neighbor Solicitation）消息
 * 
 * 
 * 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <errno.h>

#define AGING_INTERVAL 60 // 老化间隔（秒）
#define ENTRY_TIMEOUT 300 // 条目超时时间（秒）
#define REPLY_TIMEOUT 5   // 等待回复的超时时间（秒）

struct ndp_entry {
    struct in6_addr ip;
    struct ether_addr mac;
    time_t timestamp;
};

struct ndp_table {
    struct ndp_entry *entries;
    size_t size;
    size_t capacity;
};

void init_ndp_table(struct ndp_table *table, size_t capacity) {
    table->entries = malloc(capacity * sizeof(struct ndp_entry));
    table->size = 0;
    table->capacity = capacity;
}

void free_ndp_table(struct ndp_table *table) {
    free(table->entries);
}

void add_ndp_entry(struct ndp_table *table, struct in6_addr ip, struct ether_addr mac) {
    time_t now = time(NULL);

    // 查找是否已存在
    for (size_t i = 0; i < table->size; i++) {
        if (memcmp(&table->entries[i].ip, &ip, sizeof(struct in6_addr)) == 0) {
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
        fprintf(stderr, "NDP 表已满\n");
    }
}

void remove_ndp_entry(struct ndp_table *table, size_t index) {
    if (index < table->size) {
        for (size_t i = index; i < table->size - 1; i++) {
            table->entries[i] = table->entries[i + 1];
        }
        table->size--;
    }
}

int send_ns_request(struct in6_addr ip, const char *iface) {
    int sockfd;
    struct sockaddr_in6 dst_addr;
    struct icmp6_hdr icmp6_hdr;
    struct timeval timeout = {REPLY_TIMEOUT, 0};

    sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (sockfd < 0) {
        perror("socket 失败");
        return -1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin6_family = AF_INET6;
    dst_addr.sin6_addr = ip;

    memset(&icmp6_hdr, 0, sizeof(icmp6_hdr));
    icmp6_hdr.icmp6_type = ND_NEIGHBOR_SOLICIT;
    icmp6_hdr.icmp6_code = 0;

    if (sendto(sockfd, &icmp6_hdr, sizeof(icmp6_hdr), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
        perror("sendto 失败");
        close(sockfd);
        return -1;
    }

    char buf[1024];
    if (recv(sockfd, buf, sizeof(buf), 0) < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            printf("没有收到回复\n");
        } else {
            perror("recv 失败");
        }
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

void age_ndp_entries(struct ndp_table *table, const char *iface) {
    time_t now = time(NULL);

    for (size_t i = 0; i < table->size; i++) {
        if (now - table->entries[i].timestamp > ENTRY_TIMEOUT) {
            char ip_str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &table->entries[i].ip, ip_str, sizeof(ip_str));
            printf("发送邻居请求以验证 NDP 条目: %s\n", ip_str);
            if (send_ns_request(table->entries[i].ip, iface) < 0) {
                printf("删除过期的 NDP 条目: %s\n", ip_str);
                remove_ndp_entry(table, i);
                i--; // 调整索引以检查移动后的条目
            } else {
                table->entries[i].timestamp = now; // 更新时间戳
            }
        }
    }
}

void print_ndp_table(struct ndp_table *table) {
    printf("NDP 表:\n");
    for (size_t i = 0; i < table->size; i++) {
        char ip_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &table->entries[i].ip, ip_str, sizeof(ip_str));
        printf("%s -> %s\n", ip_str, ether_ntoa(&table->entries[i].mac));
    }
}

int main() {
    struct ndp_table table;
    init_ndp_table(&table, 1024);

    struct in6_addr ip1, ip2;
    struct ether_addr mac1, mac2;

    inet_pton(AF_INET6, "fe80::1", &ip1);
    inet_pton(AF_INET6, "fe80::2", &ip2);
    ether_aton_r("00:11:22:33:44:55", &mac1);
    ether_aton_r("66:77:88:99:aa:bb", &mac2);

    add_ndp_entry(&table, ip1, mac1);
    add_ndp_entry(&table, ip2, mac2);

    const char *iface = "eth0";

    while (1) {
        print_ndp_table(&table);
        age_ndp_entries(&table, iface);
        sleep(AGING_INTERVAL);
    }

    free_ndp_table(&table);
    return 0;
}