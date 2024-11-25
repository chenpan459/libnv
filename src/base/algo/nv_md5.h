
#ifndef _NV_MD5_H_INCLUDED_
#define _NV_MD5_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"
int nv_calculate_file_md5(const char *filename, char *output);
void nv_MD5(const char *input, char *output);
int nv_md5_main();

#ifdef __cplusplus
}
#endif

#endif 


