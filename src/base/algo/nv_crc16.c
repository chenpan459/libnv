

#include "nv_crc16.h"
// CRC16-CCITT (initial value 0xFFFF)
// 计算给定数据的16位CRC校验值
uint16_t nv_crc16_compute(uint8_t *data, uint16_t length) {
    // 初始化CRC校验值为0xFFFF
    uint16_t crc = 0xFFFF;
    // 用于循环的变量i
    uint8_t i;

    // 遍历数据数组
    while (length--) {
        // 将当前数据字节与CRC校验值进行异或操作
        crc ^= *(data++);
        // 对每个数据字节进行8次处理
        for (i = 0; i < 8; i++) {
            // 如果CRC校验值的最低位为1
            if (crc & 0x0001) {
                // 将CRC校验值右移1位，并与多项式0x8408进行异或操作
                crc = (crc >> 1) ^ 0x8408;
            } else {
                // 如果最低位为0，则仅将CRC校验值右移1位
                crc = crc >> 1;
            }
        }
    }
    // 返回计算得到的CRC校验值
    return crc;
}




