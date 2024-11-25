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






int nv_poll_main() {
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

    // 初始化 poll 结构
    nv_poll_t *poll_obj = nv_poll_init();
    if (!poll_obj) {
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 将监听 socket 添加到 poll 的集合
    nv_poll_add_fd(poll_obj, listen_fd, POLLIN);

    for (;;) {
        // 执行 poll 操作
        int retval = nv_poll_wait(poll_obj, -1);
        if (retval == -1) {
            perror("poll");
            nv_poll_destroy(poll_obj);
            close(listen_fd);
            return EXIT_FAILURE;
        }

        // 检查监听 socket 是否有新连接
        if (poll_obj->fds[0].revents & POLLIN) {
            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
            if (conn_fd == -1) {
                perror("accept");
            } else {
                printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // 将新的连接 socket 添加到 poll 的集合
                nv_poll_add_fd(poll_obj, conn_fd, POLLIN);
            }
        }

        // 检查其他 socket 是否有数据可读
        for (int i = 1; i < poll_obj->nfds; i++) {
            if (poll_obj->fds[i].revents & POLLIN) {
                char buffer[256];
                ssize_t count = read(poll_obj->fds[i].fd, buffer, sizeof(buffer));
                if (count == -1) {
                    perror("read");
                    nv_poll_remove_fd(poll_obj, poll_obj->fds[i].fd);
                    close(poll_obj->fds[i].fd);
                } else if (count == 0) {
                    // 对方关闭了连接
                    nv_poll_remove_fd(poll_obj, poll_obj->fds[i].fd);
                    close(poll_obj->fds[i].fd);
                } else {
                    // 处理接收到的数据
                    buffer[count] = '\0';
                    printf("Received: %s\n", buffer);
                }
            }
        }
    }

    // 销毁 poll 结构
    nv_poll_destroy(poll_obj);
    close(listen_fd);

    return EXIT_SUCCESS;
}
