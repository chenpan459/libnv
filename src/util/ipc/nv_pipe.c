#include "nv_pipe.h"


nv_pipe_t* nv_pipe_create() {
    nv_pipe_t* nv_pipe = (nv_pipe_t*)malloc(sizeof(nv_pipe_t));
    if (!nv_pipe) {
        perror("NV: Failed to allocate memory for pipe");
        return NULL;
    }

    if (pipe(nv_pipe->fd) == -1) {
        perror("NV: Failed to create pipe");
        free(nv_pipe);
        return NULL;
    }

    return nv_pipe;
}

ssize_t nv_pipe_write(nv_pipe_t* pipe, const void* buf, size_t count) {
    if (!pipe) {
        perror("NV: Invalid pipe object");
        return -1;
    }

    ssize_t bytes_written = write(pipe->fd[1], buf, count);
    if (bytes_written == -1) {
        perror("NV: Failed to write to pipe");
    }
    return bytes_written;
}

ssize_t nv_pipe_read(nv_pipe_t* pipe, void* buf, size_t count) {
    if (!pipe) {
        perror("NV: Invalid pipe object");
        return -1;
    }

    ssize_t bytes_read = read(pipe->fd[0], buf, count);
    if (bytes_read == -1) {
        perror("NV: Failed to read from pipe");
    }
    return bytes_read;
}

void nv_pipe_close(nv_pipe_t* pipe) {
    if (!pipe) {
        perror("NV: Invalid pipe object");
        return;
    }
    close(pipe->fd[0]);
    close(pipe->fd[1]);
    free(pipe);
}




int nv_pipe_main() {
    
    nv_pipe_t* pipe = nv_pipe_create();
    if (!pipe) {
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("NV: Failed to fork");
        nv_pipe_close(pipe);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // 子进程：从管道读取数据
        char buffer[100];
        close(pipe->fd[1]); // 关闭写端
        ssize_t bytes_read = nv_pipe_read(pipe, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // 确保字符串以空字符结尾
            printf("Child received: %s\n", buffer);
        }
        printf("Closing read end of pipe, fd: %d\n", pipe->fd[1]);
        nv_pipe_close(pipe);
    } else {
        // 父进程：向管道写入数据
        const char* message = "Hello from parent!";
        close(pipe->fd[0]); // 关闭读端
        nv_pipe_write(pipe, message, strlen(message));

        printf("Closing write end of pipe, fd: %d\n", pipe->fd[0]);
        nv_pipe_close(pipe);
    }

    return EXIT_SUCCESS;
}
