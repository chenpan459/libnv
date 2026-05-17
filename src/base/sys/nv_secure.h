#ifndef _NV_SECURE_H_INCLUDED_
#define _NV_SECURE_H_INCLUDED_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 常量时间比较，缓解时序侧信道 */
int nv_secure_mem_equal(const void *a, const void *b, size_t len);
int nv_secure_str_equal(const char *a, const char *b);

/* 弱口令检测（Telnet/CLI 启动校验） */
int nv_secure_is_weak_password(const char *password);

#ifdef __cplusplus
}
#endif

#endif
