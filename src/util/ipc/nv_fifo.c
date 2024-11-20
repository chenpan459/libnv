#include "nv_fifo.h"



fifo_t* nv_fifo_create(const char* name) {
    fifo_t* fifo = (fifo_t*)malloc(sizeof(fifo_t));
    if (!fifo) {
        perror("NV: Failed to allocate memory for FIFO");
        return NULL;
    }

    // 创建有名管道
    if (mkfifo(name, 0666) == -1) {
        perror("NV: Failed to create FIFO");
      //  free(fifo);
      //  return NULL;
    }

    fifo->fd = -1; // 初始时文件描述符设为-1
    return fifo;
}

fifo_t* nv_fifo_open(fifo_t* fifo, const char* name, int mode, int nonblock) {
    if (!fifo) {
        perror("NV: Invalid FIFO object");
        return NULL;
    }
    
    int flags = mode;
    if (nonblock) {
      //  flags |= O_NONBLOCK; // 添加非阻塞标志
    }
    
printf("[%s-%d] name:%s\n", __func__,__LINE__,name);
    // 打开有名管道
    fifo->fd = open(name, flags);
    printf("[%s-%d]\n", __func__,__LINE__);
    if (fifo->fd == -1) {
        printf("[%s-%d]\n", __func__,__LINE__);
        perror("NV: Failed to open FIFO");
        free(fifo);
        return NULL;
    }
printf("[%s-%d]\n", __func__,__LINE__);
    return fifo;
}

ssize_t nv_fifo_write(fifo_t* fifo, const void* buf, size_t count) {
    if (!fifo || fifo->fd == -1) {
        perror("NV: Invalid FIFO or not opened");
        return -1;
    }

    ssize_t bytes_written = write(fifo->fd, buf, count);
    if (bytes_written == -1) {
        perror("NV: Failed to write to FIFO");
    }
    return bytes_written;
}

ssize_t nv_fifo_read(fifo_t* fifo, void* buf, size_t count) {
    if (!fifo || fifo->fd == -1) {
        perror("NV: Invalid FIFO or not opened");
        return -1;
    }

    ssize_t bytes_read = read(fifo->fd, buf, count);
    if (bytes_read == -1) {
        perror("NV: Failed to read from FIFO");
    }
    return bytes_read;
}

void nv_fifo_close(fifo_t* fifo) {
    if (!fifo) {
        perror("NV: Invalid FIFO object");
        return;
    }

    if (fifo->fd != -1 && close(fifo->fd) == -1) {
        perror("NV: Failed to close FIFO");
    }
    free(fifo);
}

void nv_fifo_unlink(const char* name) {
    if (unlink(name) == -1) {
        perror("NV: Failed to unlink FIFO");
    }
}



int nv_fifo_main() {
    

    const char* fifo_name = FIFO_NAME;
    const char* message = "Hello, FIFO!";
    char buffer[100];
    
    nv_fifo_unlink(fifo_name);
    // 创建有名管道
    fifo_t* write_fifo = nv_fifo_create(fifo_name);
    if (!write_fifo) {
        return EXIT_FAILURE;
    }

    // 打开有名管道进行写操作
    if (!nv_fifo_open(write_fifo, fifo_name, O_RDWR,1)) {
        nv_fifo_unlink(fifo_name); // 发生错误时删除管道
        return EXIT_FAILURE;
    }

    // 打开有名管道进行读操作
    fifo_t* read_fifo = nv_fifo_open(nv_fifo_create(fifo_name), fifo_name, O_RDWR,1);
    if (!read_fifo) {
        nv_fifo_close(write_fifo);
        nv_fifo_unlink(fifo_name);
        return EXIT_FAILURE;
    }

    // 向有名管道写入数据
    if (nv_fifo_write(write_fifo, message, strlen(message)) == -1) {
        nv_fifo_close(write_fifo);
        nv_fifo_close(read_fifo);
        nv_fifo_unlink(fifo_name);
        return EXIT_FAILURE;
    }

    // 从有名管道读取数据
    ssize_t bytes_read = nv_fifo_read(read_fifo, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // 确保字符串以空字符结尾
        printf("Received from FIFO: %s\n", buffer);
    }

    // 关闭有名管道
    nv_fifo_close(write_fifo);
    nv_fifo_close(read_fifo);

    // 删除有名管道
    nv_fifo_unlink(fifo_name);


    return EXIT_SUCCESS;
}
