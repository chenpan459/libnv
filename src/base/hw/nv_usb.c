#include "nv_usb.h"



// 初始化 libusb 库
int nv_usb_init() {
    return libusb_init(NULL);
}

// 打开 USB 设备
// 定义一个函数，用于打开指定厂商ID和产品ID的USB设备
nv_usb_t* nv_usb_open(uint16_t vendor_id, uint16_t product_id) {
    // 动态分配内存用于存储USB设备信息
    nv_usb_t* usb = (nv_usb_t*)malloc(sizeof(nv_usb_t));
    // 检查内存分配是否成功
    if (!usb) {
        // 如果内存分配失败，输出错误信息
        perror("NV: Failed to allocate memory for USB object");
        // 返回NULL表示失败
        return NULL;
    }

    // 定义一个指针用于存储设备列表
    libusb_device **devs;
    // 获取USB设备列表，cnt为设备数量
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    // 检查设备列表获取是否成功
    if (cnt < 0) {
        // 如果获取设备列表失败，输出错误信息
        perror("NV: Failed to get device list");
        // 释放之前分配的内存
        free(usb);
        // 返回NULL表示失败
        return NULL;
    }

    // 遍历设备列表
    for (ssize_t i = 0; i < cnt; i++) {
        // 定义一个结构体用于存储设备描述符
        struct libusb_device_descriptor desc;
        // 获取设备的描述符
        int r = libusb_get_device_descriptor(devs[i], &desc);
        // 检查获取描述符是否成功
        if (r < 0) {
            // 如果获取描述符失败，输出错误信息
            perror("NV: Failed to get device descriptor");
            continue;
        }

        if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
            r = libusb_open(devs[i], &usb->handle);
            if (r != 0) {
                perror("NV: Failed to open USB device");
                libusb_free_device_list(devs, 1);
                free(usb);
                return NULL;
            }

            // 可能需要配置设备
            if (desc.bNumConfigurations > 0) {
                struct libusb_config_descriptor *config;
                r = libusb_get_config_descriptor(devs[i], 0, &config);
                if (r == 0) {
                    for (int j = 0; j < config->bNumInterfaces; j++) {
                        const struct libusb_interface *inter = &config->interface[j];
                        for (int k = 0; k < inter->num_altsetting; k++) {
                            const struct libusb_interface_descriptor *interdesc = &inter->altsetting[k];
                            for (int l = 0; l < interdesc->bNumEndpoints; l++) {
                                const struct libusb_endpoint_descriptor *epdesc = &interdesc->endpoint[l];
                                if (epdesc->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                                    usb->endpoint_in = epdesc->bEndpointAddress;
                                } else {
                                    usb->endpoint_out = epdesc->bEndpointAddress;
                                }
                            }
                        }
                    }
                    libusb_free_config_descriptor(config);
                }
            }

            libusb_free_device_list(devs, 1);
            return usb;
        }
    }

    libusb_free_device_list(devs, 1);
    free(usb);
    return NULL;
}

// 关闭 USB 设备
void nv_usb_close(nv_usb_t* usb) {
    if (usb) {
        libusb_close(usb->handle);
        free(usb);
    }
}

// 从 USB 设备读取数据
ssize_t nv_usb_read(nv_usb_t* usb, char* data, size_t size) {
    if (!usb) {
        return -1;
    }

    int transferred;
    int r = libusb_interrupt_transfer(usb->handle, usb->endpoint_in, (unsigned char*)data, size, &transferred, 0);
    if (r == 0) {
        return transferred;
    } else {
        perror("NV: Failed to read from USB device");
        return -1;
    }
}

// 向 USB 设备写入数据
// 定义函数nv_usb_write，用于向USB设备写入数据
// 参数usb是指向nv_usb_t结构体的指针，data是要写入的数据，size是数据的长度
// 返回值为ssize_t类型，表示写入的字节数，若出错则返回-1
ssize_t nv_usb_write(nv_usb_t* usb, const char* data, size_t size) {
    // 检查usb指针是否为空，若为空则返回-1表示错误
    if (!usb) {
        return -1;
    }

    // 定义变量transferred用于存储实际传输的字节数
    int transferred;
    // 调用libusb_interrupt_transfer函数进行中断传输
    // 参数分别为USB设备句柄、输出端点、数据指针、数据长度、实际传输字节数的指针、超时时间
    int r = libusb_interrupt_transfer(usb->handle, usb->endpoint_out, (unsigned char*)data, size, &transferred, 0);
    // 检查libusb_interrupt_transfer的返回值，若为0表示传输成功
    if (r == 0) {
        // 返回实际传输的字节数
        return transferred;
    } else {
        // 若传输失败，打印错误信息
        perror("NV: Failed to write to USB device");
        // 返回-1表示错误
        return -1;
    }
}











int nv_usb_main() {
    uint16_t vendor_id = 0x1234; // 修改为你的设备的供应商 ID
    uint16_t product_id = 0x5678; // 修改为你的设备的产品 ID

    // 初始化 libusb 库
    if (nv_usb_init() != 0) {
        return EXIT_FAILURE;
    }

    // 打开 USB 设备
    nv_usb_t* usb = nv_usb_open(vendor_id, product_id);
    if (!usb) {
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    // 发送数据
    const char* data_to_send = "Hello, USB!";
    ssize_t bytes_written = nv_usb_write(usb, data_to_send, strlen(data_to_send));
    if (bytes_written == -1) {
        perror("NV: Failed to write to USB device");
        nv_usb_close(usb);
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    printf("Sent: %s\n", data_to_send);

    // 接收数据
    char buffer[256];
    ssize_t bytes_read = nv_usb_read(usb, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("NV: Failed to read from USB device");
        nv_usb_close(usb);
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0'; // 确保字符串以空字符结尾
    printf("Received: %s\n", buffer);

    // 关闭 USB 设备
    nv_usb_close(usb);

    // 退出 libusb 库
    libusb_exit(NULL);

    return EXIT_SUCCESS;
}
