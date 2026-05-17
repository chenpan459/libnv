#include "nv_socket_poll.h"




// 初始化 poll 结构
nv_poll_t* nv_poll_init() {
    nv_poll_t *poll_obj = (nv_poll_t*)malloc(sizeof(nv_poll_t));
    if (!poll_obj) {
        perror("NV: Failed to allocate memory for poll object");
        return NULL;
    }

    poll_obj->nfds = 0;

    return poll_obj;
}

// 添加文件描述符到 poll 的集合
int nv_poll_add_fd(nv_poll_t *poll_obj, int fd, int events) {
    if (!poll_obj || poll_obj->nfds >= NV_POLL_MAX_FD) {
        return -1;
    }

    poll_obj->fds[poll_obj->nfds].fd = fd;
    poll_obj->fds[poll_obj->nfds].events = events;
    poll_obj->nfds++;

    return 0;
}

// 从 poll 的集合中移除文件描述符
int nv_poll_remove_fd(nv_poll_t *poll_obj, int fd) {
    if (!poll_obj) {
        return -1;
    }

    for (int i = 0; i < poll_obj->nfds; i++) {
        if (poll_obj->fds[i].fd == fd) {
            for (int j = i; j < poll_obj->nfds - 1; j++) {
                poll_obj->fds[j] = poll_obj->fds[j + 1];
            }
            poll_obj->nfds--;
            return 0;
        }
    }

    return -1;
}

// 执行 poll 操作
int nv_poll_wait(nv_poll_t *poll_obj, int timeout) {
    if (!poll_obj) {
        return -1;
    }

    int retval = poll(poll_obj->fds, poll_obj->nfds, timeout);
    if (retval == -1) {
        perror("NV: poll failed");
        return -1;
    }

    return retval;
}

// 销毁 poll 结构
void nv_poll_destroy(nv_poll_t *poll_obj) {
    if (poll_obj) {
        free(poll_obj);
    }
}
