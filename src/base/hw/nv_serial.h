#ifndef _NV_SERIAL_H_INCLUDED_
#define _NV_SERIAL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"

/**********************************
 * 
1、打开串口设备：
使用 nv_serial_open 函数打开指定的串口设备。
2、配置串口参数：
使用 nv_serial_configure 函数配置串口的波特率、数据位、停止位和校验位。
3、发送数据：
使用 nv_serial_write 函数向串口发送数据。
4、接收数据：
使用 nv_serial_read 函数从串口接收数据。
5、关闭串口：
使用 nv_serial_close 函数关闭串口并释放相关资源。
***********************************************/

typedef struct {
    int fd;
    struct termios options;
} nv_serial_t;


nv_serial_t* nv_serial_open(const char* portname) ;
// 配置串口
int nv_serial_configure(nv_serial_t* serial, int baud_rate, int data_bits, int stop_bits, char parity) ;
// 写入串口
ssize_t nv_serial_write(nv_serial_t* serial, const char* data, size_t size) ;

// 读取串口
ssize_t nv_serial_read(nv_serial_t* serial, char* data, size_t size) ;
// 关闭串口
void nv_serial_close(nv_serial_t* serial) ;



int nv_serial_main() ;


#ifdef __cplusplus
}
#endif

#endif
