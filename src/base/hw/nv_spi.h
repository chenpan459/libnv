#ifndef _NV_SPI_H_INCLUDED_
#define _NV_SPI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>



typedef struct {
    int fd;
    uint8_t mode;
    uint8_t bits_per_word;
    uint32_t speed;
} nv_spi_t;


nv_spi_t* nv_spi_open(const char* spi_device) ;
// 配置 SPI 设备
int nv_spi_configure(nv_spi_t* spi, uint8_t mode, uint8_t bits_per_word, uint32_t speed) ;
// 进行 SPI 数据传输
ssize_t nv_spi_transfer(nv_spi_t* spi, const uint8_t* tx_buf, uint8_t* rx_buf, size_t len) ;
// 关闭 SPI 设备
void nv_spi_close(nv_spi_t* spi) ;




int nv_spi_main() ;




#ifdef __cplusplus
}
#endif

#endif