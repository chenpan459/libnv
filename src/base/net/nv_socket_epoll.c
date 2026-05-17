#include "nv_socket_epoll.h"




// 初始化 epoll 实例
nv_epoll_t* nv_epoll_create() {
    nv_epoll_t *epoll_obj = (nv_epoll_t*)malloc(sizeof(nv_epoll_t));
    if (!epoll_obj) {
        perror("NV: Failed to allocate memory for epoll object");
        return NULL;
    }

    epoll_obj->epoll_fd = epoll_create1(0);
    if (epoll_obj->epoll_fd == NV_EPOLL_ERR) {
        perror("NV: Failed to create epoll instance");
        free(epoll_obj);
        return NULL;
    }

    epoll_obj->events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAX_EVENTS);
    if (!epoll_obj->events) {
        perror("NV: Failed to allocate memory for events array");
        close(epoll_obj->epoll_fd);
        free(epoll_obj);
        return NULL;
    }

    return epoll_obj;
}

// 添加文件描述符到 epoll 实例
int nv_epoll_add(nv_epoll_t *epoll_obj, int fd, uint32_t events) {
    if (!epoll_obj) {
        return NV_EPOLL_ERR;
    }

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_obj->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == NV_EPOLL_ERR) {
        perror("NV: Failed to add file descriptor to epoll");
        return NV_EPOLL_ERR;
    }

    return NV_EPOLL_SUCCESS;
}

// 修改文件描述符在 epoll 实例中的事件
int nv_epoll_mod(nv_epoll_t *epoll_obj, int fd, uint32_t events) {
    if (!epoll_obj) {
        return NV_EPOLL_ERR;
    }

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_obj->epoll_fd, EPOLL_CTL_MOD, fd, &ev) == NV_EPOLL_ERR) {
        perror("NV: Failed to modify file descriptor in epoll");
        return NV_EPOLL_ERR;
    }

    return NV_EPOLL_SUCCESS;
}

// 从 epoll 实例中移除文件描述符
int nv_epoll_del(nv_epoll_t *epoll_obj, int fd) {
    if (!epoll_obj) {
        return NV_EPOLL_ERR;
    }

    if (epoll_ctl(epoll_obj->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == NV_EPOLL_ERR) {
        perror("NV: Failed to remove file descriptor from epoll");
        return NV_EPOLL_ERR;
    }

    return NV_EPOLL_SUCCESS;
}

// 等待事件发生
int nv_epoll_wait(nv_epoll_t *epoll_obj, int timeout) {
    if (!epoll_obj) {
        return NV_EPOLL_ERR;
    }

    int nfds = epoll_wait(epoll_obj->epoll_fd, epoll_obj->events, MAX_EVENTS, timeout);
    if (nfds == NV_EPOLL_ERR) {
        perror("NV: Failed to wait for events");
    }

    return nfds;
}

// 销毁 epoll 实例
void nv_epoll_destroy(nv_epoll_t *epoll_obj) {
    if (!epoll_obj) {
        return;
    }

    close(epoll_obj->epoll_fd);
    free(epoll_obj->events);
    free(epoll_obj);
}
