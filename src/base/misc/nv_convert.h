#ifndef _NV_BCD_H_INCLUDED_
#define _NV_BCD_H_INCLUDED_



#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>
unsigned char int_to_bcd(int value) ;
// 将字符串转换为 float 类型
float str_to_float(const char *str);

void float_to_str(float value, char *str, size_t size);

#ifdef __cplusplus
}
#endif
#endif