#include "nv_serial.h"



// 打开串口设备
nv_serial_t* nv_serial_open(const char* portname) {
    nv_serial_t* serial = (nv_serial_t*)malloc(sizeof(nv_serial_t));
    if (!serial) {
        perror("NV: Failed to allocate memory for serial object");
        return NULL;
    }

    // 打开串口设备
    serial->fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial->fd == -1) {
        perror("NV: Failed to open serial port");
        free(serial);
        return NULL;
    }

    // 获取当前串口配置
    tcgetattr(serial->fd, &serial->options);

    return serial;
}

// 配置串口
int nv_serial_configure(nv_serial_t* serial, int baud_rate, int data_bits, int stop_bits, char parity) {
    if (!serial) {
        return -1;
    }

    // 设置波特率
    cfsetispeed(&serial->options, baud_rate);
    cfsetospeed(&serial->options, baud_rate);

    // 设置数据位
    serial->options.c_cflag &= ~CSIZE;
    switch (data_bits) {
        case 5:
            serial->options.c_cflag |= CS5;
            break;
        case 6:
            serial->options.c_cflag |= CS6;
            break;
        case 7:
            serial->options.c_cflag |= CS7;
            break;
        case 8:
            serial->options.c_cflag |= CS8;
            break;
        default:
            perror("NV: Invalid number of data bits");
            return -1;
    }

    // 设置停止位
    if (stop_bits == 1) {
        serial->options.c_cflag &= ~CSTOPB;
    } else if (stop_bits == 2) {
        serial->options.c_cflag |= CSTOPB;
    } else {
        perror("NV: Invalid number of stop bits");
        return -1;
    }

    // 设置校验位
    serial->options.c_cflag &= ~PARENB;
    if (parity == 'N') {
        // 无校验
    } else if (parity == 'O') {
        // 奇校验
        serial->options.c_cflag |= PARENB;
        serial->options.c_cflag |= PARODD;
    } else if (parity == 'E') {
        // 偶校验
        serial->options.c_cflag |= PARENB;
        serial->options.c_cflag &= ~PARODD;
    } else {
        perror("NV: Invalid parity");
        return -1;
    }

    // 启用接收器，设置为原始输入和原始输出
    serial->options.c_cflag |= CREAD | CLOCAL;
    cfmakeraw(&serial->options);

    // 应用配置
    tcsetattr(serial->fd, TCSANOW, &serial->options);

    return 0;
}

// 写入串口
ssize_t nv_serial_write(nv_serial_t* serial, const char* data, size_t size) {
    if (!serial) {
        return -1;
    }

    return write(serial->fd, data, size);
}

// 读取串口
ssize_t nv_serial_read(nv_serial_t* serial, char* data, size_t size) {
    if (!serial) {
        return -1;
    }

    return read(serial->fd, data, size);
}

// 关闭串口
void nv_serial_close(nv_serial_t* serial) {
    if (serial) {
        close(serial->fd);
        free(serial);
    }
}



int nv_serial_main() {
    const char* portname = "/dev/ttyS1"; // 修改为你的串口设备文件
    nv_serial_t* serial = nv_serial_open(portname);
    if (!serial) {
        return EXIT_FAILURE;
    }

    // 配置串口参数：波特率为9600，数据位为8，停止位为1，无校验
    if (nv_serial_configure(serial, B9600, 8, 1, 'N') == -1) {
        nv_serial_close(serial);
        return EXIT_FAILURE;
    }

    // 发送数据
    const char* data_to_send = "Hello, Serial Port!";
    ssize_t bytes_written = nv_serial_write(serial, data_to_send, strlen(data_to_send));
    if (bytes_written == -1) {
        perror("NV: Failed to write to serial port");
        nv_serial_close(serial);
        return EXIT_FAILURE;
    }
    printf("Sent: %s\n", data_to_send);

    // 接收数据
    char buffer[256];
    ssize_t bytes_read = nv_serial_read(serial, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("NV: Failed to read from serial port");
        nv_serial_close(serial);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0'; // 确保字符串以空字符结尾
    printf("Received: %s\n", buffer);

    // 关闭串口
    nv_serial_close(serial);

    return EXIT_SUCCESS;
}


