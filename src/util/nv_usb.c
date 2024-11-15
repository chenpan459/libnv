#include "nv_usb.h"



// 初始化 libusb 库
int nv_usb_init() {
    return libusb_init(NULL);
}

// 打开 USB 设备
nv_usb_t* nv_usb_open(uint16_t vendor_id, uint16_t product_id) {
    nv_usb_t* usb = (nv_usb_t*)malloc(sizeof(nv_usb_t));
    if (!usb) {
        perror("NV: Failed to allocate memory for USB object");
        return NULL;
    }

    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        perror("NV: Failed to get device list");
        free(usb);
        return NULL;
    }

    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0) {
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
ssize_t nv_usb_write(nv_usb_t* usb, const char* data, size_t size) {
    if (!usb) {
        return -1;
    }

    int transferred;
    int r = libusb_interrupt_transfer(usb->handle, usb->endpoint_out, (unsigned char*)data, size, &transferred, 0);
    if (r == 0) {
        return transferred;
    } else {
        perror("NV: Failed to write to USB device");
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
