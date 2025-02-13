
#include "nv_hash.h"

// DJB2 哈希算法
// 定义一个函数nv_hash_djb2，用于计算字符串的DJB2哈希值
unsigned long nv_hash_djb2(const char *str) {
    // 初始化哈希值为5381，这是一个经验值，用于提高哈希分布的均匀性
    unsigned long hash = 5381;
    // 定义一个整型变量c，用于存储当前字符的ASCII值
    int c;

    // 使用while循环遍历字符串中的每个字符，直到遇到字符串结束符'\0'
    while ((c = *str++)) {
        // 计算哈希值，公式为hash = (hash * 33) + c
        // 其中hash << 5相当于hash * 32，加上hash相当于乘以33
        // c是当前字符的ASCII值
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    // 返回计算得到的哈希值
    return hash;
}

// 计算字符串的哈希值
unsigned long nv_hash_string(const char *str) {
    return nv_hash_djb2(str);
}



// int main() {
//     const char *test_str = "Hello, World!";
//     unsigned long hash_value = nv_hash_string(test_str);
//     printf("The hash value of \"%s\" is %lu\n", test_str, hash_value);
//     return 0;
// }
