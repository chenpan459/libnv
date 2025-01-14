
#include "nv_convert.h"
// 将整数转换为 BCD 格式
unsigned char int_to_bcd(int value) {
    unsigned char bcd = 0;
    int shift = 0;

    while (value > 0) {
        bcd |= (value % 10) << (shift * 4);
        value /= 10;
        shift++;
    }

    return bcd;
}


// 将字符串转换为 float 类型
float str_to_float(const char *str) {
    char *endptr;
    float value;

    // 使用 strtof 将字符串转换为 float
    value = strtof(str, &endptr);

    // 检查转换是否成功
    if (*endptr != '\0') {
        fprintf(stderr, "Failed to convert string to float\n");
        return 0.0f; // 或者返回一个适当的错误值
    }

    return value;
}

// 将 float 类型转换为字符串
void float_to_str(float value, char *str, size_t size) {
    // 使用 snprintf 将 float 转换为字符串
    snprintf(str, size, "%.6f", value); // 保留 6 位小数
}



