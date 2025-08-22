#include "nv_fifo.h"

fifo_t* nv_fifo_create(const char* name) {
    fifo_t* fifo = (fifo_t*)malloc(sizeof(fifo_t));
    if (!fifo) {
        perror("NV: Failed to allocate memory for FIFO");
        return NULL;
    }

    if (mkfifo(name, 0666) == -1) {
        if (errno != EEXIST) {
            perror("NV: Failed to create FIFO");
            free(fifo);
            return NULL;
        }
    }

    fifo->fd = -1;
    return fifo;
}

fifo_t* nv_fifo_open(fifo_t* fifo, const char* name, int mode, int nonblock) {
    if (!fifo) {
        perror("NV: Invalid FIFO object");
        return NULL;
    }

    int flags = mode;
    if (nonblock) {
        flags |= O_NONBLOCK;
    }

    fifo->fd = open(name, flags);
    if (fifo->fd == -1) {
        perror("NV: Failed to open FIFO");
        free(fifo);
        return NULL;
    }
    return fifo;
}

ssize_t nv_fifo_write(fifo_t* fifo, const void* buf, size_t count) {
    if (!fifo || fifo->fd == -1) {
        errno = EBADF;
        return -1;
    }
    return write(fifo->fd, buf, count);
}

ssize_t nv_fifo_read(fifo_t* fifo, void* buf, size_t count) {
    if (!fifo || fifo->fd == -1) {
        errno = EBADF;
        return -1;
    }
    return read(fifo->fd, buf, count);
}

static int nv_fifo_wait(int fd, int for_write, int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = for_write ? POLLOUT : POLLIN;
    pfd.revents = 0;
    int t = timeout_ms < 0 ? -1 : timeout_ms;
    int rc = poll(&pfd, 1, t);
    if (rc <= 0) return rc; /* 0:timeout, <0:error */
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) return -1;
    return 1;
}

ssize_t nv_fifo_write_timeout(fifo_t* fifo, const void* buf, size_t count, int timeout_ms) {
    if (!fifo || fifo->fd == -1) { errno = EBADF; return -1; }
    int rc = nv_fifo_wait(fifo->fd, 1, timeout_ms);
    if (rc <= 0) { if (rc == 0) errno = EAGAIN; return -1; }
    return write(fifo->fd, buf, count);
}

ssize_t nv_fifo_read_timeout(fifo_t* fifo, void* buf, size_t count, int timeout_ms) {
    if (!fifo || fifo->fd == -1) { errno = EBADF; return -1; }
    int rc = nv_fifo_wait(fifo->fd, 0, timeout_ms);
    if (rc <= 0) { if (rc == 0) errno = EAGAIN; return -1; }
    return read(fifo->fd, buf, count);
}

void nv_fifo_close(fifo_t* fifo) {
    if (!fifo) return;
    if (fifo->fd != -1) close(fifo->fd);
    free(fifo);
}

void nv_fifo_unlink(const char* name) {
    if (unlink(name) == -1 && errno != ENOENT) {
        perror("NV: Failed to unlink FIFO");
    }
}

int nv_fifo_main() {
    const char* fifo_name = "./my_fifo";
    const char* message = "Hello, FIFO!";
    char buffer[100];

    nv_fifo_unlink(fifo_name);

    fifo_t* write_fifo = nv_fifo_create(fifo_name);
    if (!write_fifo) return EXIT_FAILURE;

    if (!nv_fifo_open(write_fifo, fifo_name, O_WRONLY | O_NONBLOCK, 0)) {
        nv_fifo_unlink(fifo_name);
        return EXIT_FAILURE;
    }

    fifo_t* read_fifo = nv_fifo_create(fifo_name);
    if (!read_fifo) { nv_fifo_close(write_fifo); nv_fifo_unlink(fifo_name); return EXIT_FAILURE; }
    if (!nv_fifo_open(read_fifo, fifo_name, O_RDONLY | O_NONBLOCK, 0)) {
        nv_fifo_close(write_fifo); nv_fifo_close(read_fifo); nv_fifo_unlink(fifo_name); return EXIT_FAILURE;
    }

    if (nv_fifo_write_timeout(write_fifo, message, strlen(message), 1000) == -1) {
        nv_fifo_close(write_fifo); nv_fifo_close(read_fifo); nv_fifo_unlink(fifo_name); return EXIT_FAILURE;
    }

    ssize_t bytes_read = nv_fifo_read_timeout(read_fifo, buffer, sizeof(buffer) - 1, 1000);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Received from FIFO: %s\n", buffer);
    }

    nv_fifo_close(write_fifo);
    nv_fifo_close(read_fifo);
    nv_fifo_unlink(fifo_name);

    return EXIT_SUCCESS;
}
