
#include "nv_hash.h"

// DJB2 哈希算法
unsigned long nv_hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

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
