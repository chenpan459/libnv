#ifndef _NV_BASE64_H_INCLUDED_
#define _NV_BASE64_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"


// Base64编码函数
char *nv_base64_encode(const unsigned char *input, size_t length);

// Base64解码函数
int nv_base64_decode(const char *input, unsigned char **output);

int nv_base64_main();

#ifdef __cplusplus
}
#endif

#endif