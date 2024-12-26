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

#define AGING_INTERVAL 10//60 // 老化间隔（秒）
#define ENTRY_TIMEOUT 5//300 // 条目超时时间（秒）
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
    char command[256];
    snprintf(command, sizeof(command), "arping -c 1 -I %s %s", iface, inet_ntoa(ip));
    return system(command);
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

    inet_aton("192.168.1.10", &ip1);
    //inet_aton("192.168.1.2", &ip2);
    ether_aton_r("00:2b:67:12:57:4e", &mac1);
    //ether_aton_r("66:77:88:99:aa:bb", &mac2);

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