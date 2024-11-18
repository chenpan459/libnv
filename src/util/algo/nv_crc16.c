

#include "nv_crc16.h"
// CRC16-CCITT (initial value 0xFFFF)
uint16_t nv_crc16_compute(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (length--) {
        crc ^= *(data++);
        for (i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0x8408;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}




