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







int nv_epoll_main() {
    int listen_fd, conn_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 创建监听 socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // 绑定地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // 选择一个端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 开始监听
    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 初始化 epoll 实例
    nv_epoll_t *epoll_obj = nv_epoll_create();
    if (!epoll_obj) {
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 将监听 socket 添加到 epoll 实例
    if (nv_epoll_add(epoll_obj, listen_fd, EPOLLIN) == NV_EPOLL_ERR) {
        nv_epoll_destroy(epoll_obj);
        close(listen_fd);
        return EXIT_FAILURE;
    }

    for (;;) {
        // 等待事件发生
        int nfds = nv_epoll_wait(epoll_obj, -1);
        if (nfds == NV_EPOLL_ERR) {
            nv_epoll_destroy(epoll_obj);
            close(listen_fd);
            return EXIT_FAILURE;
        }

        for (int n = 0; n < nfds; ++n) {
            if (epoll_obj->events[n].data.fd == listen_fd) {
                // 新的连接
                conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
                if (conn_fd == -1) {
                    perror("accept");
                } else {
                    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    // 将新的连接 socket 添加到 epoll 实例
                    nv_epoll_add(epoll_obj, conn_fd, EPOLLIN);
                }
            } else {
                // 处理其他 I/O 事件
                if (epoll_obj->events[n].events & EPOLLIN) {
                    char buffer[256];
                    ssize_t count = read(epoll_obj->events[n].data.fd, buffer, sizeof(buffer));
                    if (count == -1) {
                        perror("read");
                        nv_epoll_del(epoll_obj, epoll_obj->events[n].data.fd);
                        close(epoll_obj->events[n].data.fd);
                    } else if (count == 0) {
                        // 对方关闭了连接
                        nv_epoll_del(epoll_obj, epoll_obj->events[n].data.fd);
                        close(epoll_obj->events[n].data.fd);
                    } else {
                        // 处理接收到的数据
                        buffer[count] = '\0';
                        printf("Received: %s\n", buffer);
                    }
                }
            }
        }
    }

    // 销毁 epoll 实例
    nv_epoll_destroy(epoll_obj);
    close(listen_fd);

    return EXIT_SUCCESS;
}
