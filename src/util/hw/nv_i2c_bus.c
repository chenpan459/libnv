#include "nv_i2c_bus.h"


// 打开 I2C 设备
nv_i2c_t* nv_i2c_open(const char* i2c_device) {
    nv_i2c_t* i2c = (nv_i2c_t*)malloc(sizeof(nv_i2c_t));
    if (!i2c) {
        perror("NV: Failed to allocate memory for I2C object");
        return NULL;
    }

    // 打开 I2C 设备文件
    i2c->fd = open(i2c_device, O_RDWR);
    if (i2c->fd == -1) {
        perror("NV: Failed to open I2C device");
        free(i2c);
        return NULL;
    }

    return i2c;
}

// 设置 I2C 从设备地址
int nv_i2c_set_address(nv_i2c_t* i2c, int addr) {
    if (!i2c) {
        return -1;
    }

    // 设置 I2C 从设备地址
    if (ioctl(i2c->fd, I2C_SLAVE, addr) == -1) {
        perror("NV: Failed to set I2C address");
        return -1;
    }

    i2c->addr = addr;

    return 0;
}

// 写入 I2C 设备
ssize_t nv_i2c_write(nv_i2c_t* i2c, const char* data, size_t size) {
    if (!i2c) {
        return -1;
    }

    return write(i2c->fd, data, size);
}

// 读取 I2C 设备
ssize_t nv_i2c_read(nv_i2c_t* i2c, char* data, size_t size) {
    if (!i2c) {
        return -1;
    }

    return read(i2c->fd, data, size);
}

// 关闭 I2C 设备
void nv_i2c_close(nv_i2c_t* i2c) {
    if (i2c) {
        close(i2c->fd);
        free(i2c);
    }
}






int nv_i2c_main() {
    const char* i2c_device = "/dev/i2c-1"; // 修改为你的 I2C 设备文件
    int i2c_addr = 0x50; // 修改为你的从设备地址

    // 打开 I2C 设备
    nv_i2c_t* i2c = nv_i2c_open(i2c_device);
    if (!i2c) {
        return EXIT_FAILURE;
    }

    // 设置从设备地址
    if (nv_i2c_set_address(i2c, i2c_addr) == -1) {
        nv_i2c_close(i2c);
        return EXIT_FAILURE;
    }

    // 发送数据
    const char* data_to_send = "Hello, I2C!";
    ssize_t bytes_written = nv_i2c_write(i2c, data_to_send, strlen(data_to_send));
    if (bytes_written == -1) {
        perror("NV: Failed to write to I2C device");
        nv_i2c_close(i2c);
        return EXIT_FAILURE;
    }
    printf("Sent: %s\n", data_to_send);

    // 接收数据
    char buffer[256];
    ssize_t bytes_read = nv_i2c_read(i2c, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("NV: Failed to read from I2C device");
        nv_i2c_close(i2c);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0'; // 确保字符串以空字符结尾
    printf("Received: %s\n", buffer);

    // 关闭 I2C 设备
    nv_i2c_close(i2c);

    return EXIT_SUCCESS;
}




