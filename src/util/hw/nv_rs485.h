#ifndef _NV_RS485_H_INCLUDED_
#define _NV_RS485_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"

#include <fcntl.h>
#include <termios.h>


// RS-485 通信接口结构体
typedef struct {
    int fd;
    struct termios tty;
} nv_rs485_t;

// 打开 RS-485 端口
nv_rs485_t* nv_rs485_open(const char* portname) ;

// 配置 RS-485 端口
int nv_rs485_configure(nv_rs485_t* rs485, int baudrate) ;
// 读取数据
ssize_t nv_rs485_read(nv_rs485_t* rs485, void* buffer, size_t size) ;

// 写入数据
ssize_t nv_rs485_write(nv_rs485_t* rs485, const void* buffer, size_t size) ;

// 关闭 RS-485 端口
void nv_rs485_close(nv_rs485_t* rs485) ;


#ifdef __cplusplus
}
#endif

#endif
