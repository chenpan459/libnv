#include "nv_string.h"

/**************************************
 * 用于在一个字符串中查找指定字符最后一次出现的位置
 * *****************************/
char *nv_strrchr(const char *str, int c){
     return  strrchr(str, c);
}

/**************************************
 * 字符串比较大小
 * *****************************/
int nv_strcmp(char *str1,char * str2 ) {
    return strcmp(str1, str2);
}


/**************************************
 * 字符串转大写
 * *****************************/
int nv_str_toupper(char *str) {
     int i;
    // 计算字符串的长度
    int len = strlen(str);
    // 遍历字符串中的每个字符
    for (i = 0; i < len; i++) {
        // 将每个字符转换为大写
        str[i] = toupper(str[i]);
    }
    // 打印转换后的字符串
    //printf("大写字符串: %s\n", str);

    return 0;
}

/**************************************
 * 字符串转小写
 * ********************************/
int nv_str_tolower(char *str) {
    int i;

    // 计算字符串的长度
    int len = strlen(str);
    // 遍历字符串中的每个字符
    for (i = 0; i < len; i++) {
        // 将每个字符转换为小写
        str[i] = tolower(str[i]);
    }
    // 打印转换后的字符串
    //printf("大写字符串: %s\n", str);
    return 0;
}

/********************************
 * 个不限参数的函数来格式化输出字符串
 * ***************************************/
void nv_format_string(char *buffer, size_t bufsize, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, bufsize, format, args);
    va_end(args);
}


/**************************************
 * 字符串拷贝
 * *****************************/
char* nv_strcpy(char *str1,char * str2 ) {
    return strcpy(str1, str2);
}


// 字符串连接
char* nv_strcat(char *dest, const char *src) {
    return strcat(dest, src);
}
// 获取字符串长度
size_t nv_strlen(const char *str) {
    return strlen(str);
}

// 用于不区分大小写地比较两个字符串的前 n 个字符
int strnicmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) {
        return 0;
    }

    while (n-- != 0 && tolower((unsigned char)*s1) == tolower((unsigned char)*s2)) {
        if (n == 0 || *s1 == '\0' || *s2 == '\0') {
            break;
        }
        s1++;
        s2++;
    }

    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}