
#include "nv_bcd.h"
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
