#ifndef _NV_I2C_BUS_H_INCLUDED_
#define _NV_I2C_BUS_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_util_include.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>



typedef struct {
    int fd;
    int addr;
} nv_i2c_t;


// 打开 I2C 设备
nv_i2c_t* nv_i2c_open(const char* i2c_device) ;
// 设置 I2C 从设备地址
int nv_i2c_set_address(nv_i2c_t* i2c, int addr) ;
// 写入 I2C 设备
ssize_t nv_i2c_write(nv_i2c_t* i2c, const char* data, size_t size) ;
// 读取 I2C 设备
ssize_t nv_i2c_read(nv_i2c_t* i2c, char* data, size_t size) ;
// 关闭 I2C 设备
void nv_i2c_close(nv_i2c_t* i2c) ;



int nv_i2c_main() ;

#ifdef __cplusplus
}
#endif

#endif 