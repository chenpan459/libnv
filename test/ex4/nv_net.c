#include <nv.h>
#include <nv_udp.h>
#include <nv_log.h>
#include <nv_version.h>
#include <nv_socket.h>
#include <nv_string.h>
#include <nv_mem.h>
#include <nv_sys.h>
#include <nv_thread.h>
#include <nv_udp.h>
#include <nv_net.h>


int main() {

    const char *ipv4_str = "192.168.1.1";
    uint32_t ipv4_int;
    char ipv4_buffer[INET_ADDRSTRLEN];

    // IPv4 字符串地址转整数
    if (nv_ipv4_str_to_int(ipv4_str, &ipv4_int) == 0) {
        printf("IPv4 转换成功: %s -> %u\n", ipv4_str, ipv4_int);
    } else {
        printf("IPv4 转换失败\n");
    }

    // IPv4 整数地址转字符串
    if (nv_ipv4_int_to_str(ipv4_int, ipv4_buffer, sizeof(ipv4_buffer)) == 0) {
        printf("IPv4 转换成功: %u -> %s\n", ipv4_int, ipv4_buffer);
    } else {
        printf("IPv4 转换失败\n");
    }

    const char *ipv6_str = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    uint32_t ipv6_int_array[4];
    char ipv6_buffer[INET6_ADDRSTRLEN];

    // IPv6 字符串地址转整数数组
    if (nv_ipv6_str_to_int_array(ipv6_str, ipv6_int_array) == 0) {
        printf("IPv6 转换成功: %s -> [%u, %u, %u, %u]\n", ipv6_str, ipv6_int_array[0], ipv6_int_array[1], ipv6_int_array[2], ipv6_int_array[3]);
    } else {
        printf("IPv6 转换失败\n");
    }

    // IPv6 整数数组地址转字符串
    if (nv_ipv6_int_array_to_str(ipv6_int_array, ipv6_buffer, sizeof(ipv6_buffer)) == 0) {
        printf("IPv6 转换成功: [%u, %u, %u, %u] -> %s\n", ipv6_int_array[0], ipv6_int_array[1], ipv6_int_array[2], ipv6_int_array[3], ipv6_buffer);
    } else {
        printf("IPv6 转换失败\n");
    }

    return 0;
    
}



