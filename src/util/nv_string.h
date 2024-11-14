

#ifndef _NV_STRING_H_INCLUDED_
#define _NV_STRING_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "nv_util_include.h"


char *nv_strrchr(const char *str, int c);
int nv_str_tolower(char *str);
int nv_str_toupper(char *str);
int nv_strcmp(char *str1,char * str2 );
void nv_format_string(char *buffer, size_t bufsize, const char *format, ...);
char* nv_strcpy(char *str1,char * str2 );
void* nv_memcpy(void *dest,void * src,size_t  n ) ;
void* nv_memset(void *dest,int a,size_t  n ) ;
#ifdef __cplusplus
}
#endif

#endif