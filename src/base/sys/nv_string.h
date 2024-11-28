

#ifndef _NV_STRING_H_INCLUDED_
#define _NV_STRING_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>


char *nv_strrchr(const char *str, int c);
int nv_str_tolower(char *str);
int nv_str_toupper(char *str);
int nv_strcmp(char *str1,char * str2 );
void nv_format_string(char *buffer, size_t bufsize, const char *format, ...);
char* nv_strcpy(char *str1,char * str2 );

// 字符串连接
char* nv_strcat(char *dest, const char *src);
// 获取字符串长度
size_t nv_strlen(const char *str);

// 用于不区分大小写地比较两个字符串的前 n 个字符
int strnicmp(const char *s1, const char *s2, size_t n) ;

#ifdef __cplusplus
}
#endif

#endif