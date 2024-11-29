
#ifndef _NV_NET_H_INCLUDED_
#define _NV_NET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <arpa/inet.h>

// IPv4 字符串地址转整数
int nv_ipv4_str_to_int(const char *ip_str, uint32_t *ip_int);

// IPv4 整数地址转字符串
int nv_ipv4_int_to_str(uint32_t ip_int, char *ip_str, size_t ip_str_size);

// IPv6 字符串地址转整数数组
int nv_ipv6_str_to_int_array(const char *ip_str, uint32_t ip_int_array[4]);

// IPv6 整数数组地址转字符串
int nv_ipv6_int_array_to_str(const uint32_t ip_int_array[4], char *ip_str, size_t ip_str_size);



#ifdef __cplusplus
}
#endif

#endif
