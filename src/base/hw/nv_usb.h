
#ifndef _NV_USB_H_INCLUDED_
#define _NV_USB_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_util_include.h"

/*********************************
 * 
1、安装 libusb 库：
在基于 Debian 的系统（如 Ubuntu）上，你可以使用以下命令安装 libusb 库：
sudo apt-get install libusb-1.0-0-dev
2、在基于 Red Hat 的系统（如 CentOS 或 Fedora）上，你可以使用以下命令安装 libusb 库：
sudo yum install libusb-devel
3、在 macOS 上，你可以使用 Homebrew 安装 libusb：
brew install libusb
4、检查 libusb 是否正确安装：

5、安装完成后，你可以使用以下命令检查 libusb 头文件是否存在：
ls /usr/include/libusb-1.0/libusb.h
如果头文件存在，那么路径应该是 /usr/include/libusb-1.0/libusb.h。如果不存在，你可能需要检查安装是否成功或者头文件是否在其他位置。
******************************/


#include <libusb-1.0/libusb.h>

typedef struct {
    libusb_device_handle *handle;
    uint8_t endpoint_in;
    uint8_t endpoint_out;
} nv_usb_t;

int nv_usb_init() ;
nv_usb_t* nv_usb_open(uint16_t vendor_id, uint16_t product_id) ;
void nv_usb_close(nv_usb_t* usb) ;
ssize_t nv_usb_read(nv_usb_t* usb, char* data, size_t size) ;
ssize_t nv_usb_write(nv_usb_t* usb, const char* data, size_t size) ;




int nv_usb_main() ;


#ifdef __cplusplus
}
#endif


#endif