#include "nv_spi.h"



// 打开 SPI 设备
nv_spi_t* nv_spi_open(const char* spi_device) {
    nv_spi_t* spi = (nv_spi_t*)malloc(sizeof(nv_spi_t));
    if (!spi) {
        perror("NV: Failed to allocate memory for SPI object");
        return NULL;
    }

    // 打开 SPI 设备文件
    spi->fd = open(spi_device, O_RDWR);
    if (spi->fd == -1) {
        perror("NV: Failed to open SPI device");
        free(spi);
        return NULL;
    }

    return spi;
}

// 配置 SPI 设备
int nv_spi_configure(nv_spi_t* spi, uint8_t mode, uint8_t bits_per_word, uint32_t speed) {
    if (!spi) {
        return -1;
    }

    // 设置 SPI 模式
    if (ioctl(spi->fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("NV: Failed to set SPI mode");
        return -1;
    }
    spi->mode = mode;

    // 设置每个字的位数
    if (ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1) {
        perror("NV: Failed to set SPI bits per word");
        return -1;
    }
    spi->bits_per_word = bits_per_word;

    // 设置最大速度（Hz）
    if (ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("NV: Failed to set SPI max speed");
        return -1;
    }
    spi->speed = speed;

    return 0;
}

// 进行 SPI 数据传输
ssize_t nv_spi_transfer(nv_spi_t* spi, const uint8_t* tx_buf, uint8_t* rx_buf, size_t len) {
    if (!spi) {
        return -1;
    }

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
        .speed_hz = spi->speed,
        .bits_per_word = spi->bits_per_word,
        .cs_change = 0,
    };

    if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr) == -1) {
        perror("NV: SPI data transfer failed");
        return -1;
    }

    return len;
}

// 关闭 SPI 设备
void nv_spi_close(nv_spi_t* spi) {
    if (spi) {
        close(spi->fd);
        free(spi);
    }
}


int nv_spi_main() {
    const char* spi_device = "/dev/spidev0.0"; // 修改为你的 SPI 设备文件
    uint8_t mode = SPI_MODE_0;
    uint8_t bits_per_word = 8;
    uint32_t speed = 500000; // 500 kHz

    // 打开 SPI 设备
    nv_spi_t* spi = nv_spi_open(spi_device);
    if (!spi) {
        return EXIT_FAILURE;
    }

    // 配置 SPI 设备
    if (nv_spi_configure(spi, mode, bits_per_word, speed) == -1) {
        nv_spi_close(spi);
        return EXIT_FAILURE;
    }

    // 准备发送和接收缓冲区
    uint8_t tx_buf[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_buf[4] = {0};

    // 进行 SPI 数据传输
    ssize_t bytes_transferred = nv_spi_transfer(spi, tx_buf, rx_buf, sizeof(tx_buf));
    if (bytes_transferred == -1) {
        perror("NV: SPI data transfer failed");
        nv_spi_close(spi);
        return EXIT_FAILURE;
    }

    // 打印接收到的数据
    printf("Received: ");
    for (size_t i = 0; i < sizeof(rx_buf); i++) {
        printf("%02x ", rx_buf[i]);
    }
    printf("\n");

    // 关闭 SPI 设备
    nv_spi_close(spi);

    return EXIT_SUCCESS;
}
