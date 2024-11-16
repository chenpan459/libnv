
#ifndef _NV_MD5_H_INCLUDED_
#define _NV_MD5_H_INCLUDED_


#include "nv_util_include.h"
int nv_calculate_file_md5(const char *filename, char *output);
void nv_MD5(const char *input, char *output);

#endif 


