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
