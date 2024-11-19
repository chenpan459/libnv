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

    if (tcgetattr(rs485->fd, &rs485->tty) != 0) {
        perror("tcgetattr");
        close(rs485->fd);
        free(rs485);
        return NULL;
    }

    return rs485;
}

// 配置 RS-485 端口
int nv_rs485_configure(nv_rs485_t* rs485, int baudrate) {
    cfsetospeed(&rs485->tty, baudrate);
    cfsetispeed(&rs485->tty, baudrate);

    rs485->tty.c_cflag = (rs485->tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    rs485->tty.c_iflag &= ~IGNBRK; // disable break processing
    rs485->tty.c_lflag = 0; // no signaling chars, no echo, no canonical processing
    rs485->tty.c_oflag = 0; // no remapping, no delays
    rs485->tty.c_cc[VMIN]  = 0; // read doesn't block
    rs485->tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    rs485->tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    rs485->tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls, enable reading
    rs485->tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    rs485->tty.c_cflag |= 0;
    rs485->tty.c_cflag &= ~CSTOPB;
    rs485->tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(rs485->fd, TCSANOW, &rs485->tty) != 0) {
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

