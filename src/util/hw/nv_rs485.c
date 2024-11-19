#include "nv_rs485.h"


// 打开 RS-485 端口
nv_rs485_t* nv_rs485_open(const char* portname) {
    nv_rs485_t* rs485 = (nv_rs485_t*)malloc(sizeof(nv_rs485_t));
    if (!rs485) {
        perror("malloc");
        return NULL;
    }

    rs485->fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (rs485->fd < 0) {
        perror("open");
        free(rs485);
        return NULL;
    }

    if (tcgetattr(rs485->fd, &rs485->options) != 0) {
        perror("tcgetattr");
        close(rs485->fd);
        free(rs485);
        return NULL;
    }

    return rs485;
}

// 配置 RS-485 端口
int nv_rs485_configure(nv_rs485_t* rs485, int baudrate, int data_bits, int stop_bits, char parity) {
    cfsetospeed(&rs485->options, baudrate);
    cfsetispeed(&rs485->options, baudrate);

    rs485->options.c_cflag &= ~CSIZE;
    switch (data_bits) {
        case 6:
            rs485->options.c_cflag |= CS6;
            break;
        case 7:
            rs485->options.c_cflag |= CS7;
            break;
        case 8:
            rs485->options.c_cflag |= CS8;
            break;
        default:
            perror("Invalid number of data bits");
            return -1;
    }

    // 设置停止位
    if (stop_bits == 1) {
        rs485->options.c_cflag &= ~CSTOPB;
    } else if (stop_bits == 2) {
        rs485->options.c_cflag |= CSTOPB;
    } else {
        perror("Invalid number of stop bits");
        return -1;
    }

    // 设置校验位
    rs485->options.c_cflag &= ~PARENB;
    if (parity == 'N') {
        // 无校验
    } else if (parity == 'O') {
        // 奇校验
        rs485->options.c_cflag |= PARENB;
        rs485->options.c_cflag |= PARODD;
    } else if (parity == 'E') {
        // 偶校验
        rs485->options.c_cflag |= PARENB;
        rs485->options.c_cflag &= ~PARODD;
    } else {
        perror("Invalid parity");
        return -1;
    }

    rs485->options.c_cflag |= (CLOCAL | CREAD); // 忽略调制解调器控制线，启用接收器
    rs485->options.c_iflag &= ~(IXON | IXOFF | IXANY); // 关闭软件流控制
    rs485->options.c_lflag = 0; // 原始输入模式
    rs485->options.c_oflag = 0; // 原始输出模式

    rs485->options.c_cc[VMIN]  = 1; // 最小读取字符数
    rs485->options.c_cc[VTIME] = 5; // 读取超时（以十分之一秒为单位）

    if (tcsetattr(rs485->fd, TCSANOW, &rs485->options) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

// 读取数据
ssize_t nv_rs485_read(nv_rs485_t* rs485, void* buffer, size_t size) {
    return read(rs485->fd, buffer, size);
}

// 写入数据
ssize_t nv_rs485_write(nv_rs485_t* rs485, const void* buffer, size_t size) {
    return write(rs485->fd, buffer, size);
}

// 关闭 RS-485 端口
void nv_rs485_close(nv_rs485_t* rs485) {
    if (rs485) {
        close(rs485->fd);
        free(rs485);
    }
}




int nv_rs485_main() {
    const char* portname = "/dev/ttyS0"; // 替换为实际的 RS-485 端口
    nv_rs485_t* rs485 = nv_rs485_open(portname);
    if (!rs485) {
        return 1;
    }

    if (nv_rs485_configure(rs485, B9600, 8, 1, 'N') != 0) {
        nv_rs485_close(rs485);
        return 1;
    }

    const char* message = "Hello, RS-485!";
    nv_rs485_write(rs485, message, strlen(message));

    char buffer[100];
    ssize_t n = nv_rs485_read(rs485, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Read from RS-485: %s\n", buffer);
    } else {
        printf("No data read from RS-485\n");
    }

    nv_rs485_close(rs485);
    return 0;
}
