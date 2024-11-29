#include "nv_net.h"
#include <stdio.h>
#include <string.h>

// IPv4 字符串地址转整数
int nv_ipv4_str_to_int(const char *ip_str, uint32_t *ip_int) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) <= 0) {
        perror("inet_pton 失败");
        return -1;
    }
    *ip_int = ntohl(addr.s_addr);
    return 0;
}

// IPv4 整数地址转字符串
int nv_ipv4_int_to_str(uint32_t ip_int, char *ip_str, size_t ip_str_size) {
    struct in_addr addr;
    addr.s_addr = htonl(ip_int);
    if (inet_ntop(AF_INET, &addr, ip_str, ip_str_size) == NULL) {
        perror("inet_ntop 失败");
        return -1;
    }
    return 0;
}

// IPv6 字符串地址转整数数组
int nv_ipv6_str_to_int_array(const char *ip_str, uint32_t ip_int_array[4]) {
    struct in6_addr addr;
    if (inet_pton(AF_INET6, ip_str, &addr) <= 0) {
        perror("inet_pton 失败");
        return -1;
    }
    for (int i = 0; i < 4; i++) {
        ip_int_array[i] = ntohl(((uint32_t *)addr.s6_addr)[i]);
    }
    return 0;
}

// IPv6 整数数组地址转字符串
int nv_ipv6_int_array_to_str(const uint32_t ip_int_array[4], char *ip_str, size_t ip_str_size) {
    struct in6_addr addr;
    for (int i = 0; i < 4; i++) {
        ((uint32_t *)addr.s6_addr)[i] = htonl(ip_int_array[i]);
    }
    if (inet_ntop(AF_INET6, &addr, ip_str, ip_str_size) == NULL) {
        perror("inet_ntop 失败");
        return -1;
    }
    return 0;
}