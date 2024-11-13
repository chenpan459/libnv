

#ifndef _NV_STRING_H_INCLUDED_
#define _NV_STRING_H_INCLUDED_

#include <stdio.h>
#include <ctype.h> // 包含toupper函数
#include <string.h> // 包含strlen函数
#include <stdarg.h>

char *nv_strrchr(const char *str, int c);
int nv_str_tolower(char *str);
int nv_str_toupper(char *str);
int nv_strcmp(char *str1,char * str2 );
void format_string(char *buffer, size_t bufsize, const char *format, ...);


#endif