#include "nv_i2c_bus.h"


// 打开 I2C 设备
// 定义一个函数，用于打开I2C设备并返回一个指向nv_i2c_t结构体的指针
nv_i2c_t* nv_i2c_open(const char* i2c_device) {
    // 动态分配内存用于存储nv_i2c_t结构体
    nv_i2c_t* i2c = (nv_i2c_t*)malloc(sizeof(nv_i2c_t));
    // 检查内存分配是否成功，如果失败则打印错误信息并返回NULL
    if (!i2c) {
        perror("NV: Failed to allocate memory for I2C object");
        return NULL;
    }

    // 打开 I2C 设备文件，以读写模式打开
    i2c->fd = open(i2c_device, O_RDWR);
    // 检查文件描述符是否为-1，如果是则表示打开失败，打印错误信息，释放之前分配的内存，并返回NULL
    if (i2c->fd == -1) {
        perror("NV: Failed to open I2C device");
        free(i2c);
        return NULL;
    }

    // 如果成功打开I2C设备，返回指向nv_i2c_t结构体的指针
    return i2c;
}

// 设置 I2C 从设备地址
int nv_i2c_set_address(nv_i2c_t* i2c, int addr) {
    // 检查传入的 i2c 结构体指针是否为空
    if (!i2c) {
        // 如果为空，返回错误码 -1
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
// 定义一个函数nv_i2c_write，用于通过I2C接口写入数据
// 参数i2c是指向nv_i2c_t结构体的指针，data是要写入的数据，size是数据的长度
// 返回值为ssize_t类型，表示写入的字节数，若出错则返回-1
ssize_t nv_i2c_write(nv_i2c_t* i2c, const char* data, size_t size) {
    // 检查传入的i2c指针是否为空，如果为空则返回-1表示错误
    if (!i2c) {
        return -1;
    }

    // 调用write系统调用，将数据写入到i2c设备文件描述符i2c->fd中
    // data指向要写入的数据，size表示数据的长度
    // 返回值是实际写入的字节数，若出错则返回-1
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




