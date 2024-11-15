#include "nv_socket_select.h"




// 初始化 select 结构
nv_select_t* nv_select_init() {
    nv_select_t *select_obj = (nv_select_t*)malloc(sizeof(nv_select_t));
    if (!select_obj) {
        perror("NV: Failed to allocate memory for select object");
        return NULL;
    }

    FD_ZERO(&select_obj->read_fds);
    FD_ZERO(&select_obj->write_fds);
    FD_ZERO(&select_obj->except_fds);
    select_obj->max_fd = -1;
    select_obj->timeout.tv_sec = 0;
    select_obj->timeout.tv_usec = 0;

    return select_obj;
}

// 添加文件描述符到 select 的集合
int nv_select_add_fd(nv_select_t *select_obj, int fd, int read, int write, int except) {
    if (!select_obj || fd >= NV_SELECT_MAX_FD) {
        return -1;
    }

    if (read) {
        FD_SET(fd, &select_obj->read_fds);
    }
    if (write) {
        FD_SET(fd, &select_obj->write_fds);
    }
    if (except) {
        FD_SET(fd, &select_obj->except_fds);
    }

    if (fd > select_obj->max_fd) {
        select_obj->max_fd = fd;
    }

    return 0;
}

// 从 select 的集合中移除文件描述符
int nv_select_remove_fd(nv_select_t *select_obj, int fd) {
    if (!select_obj) {
        return -1;
    }

    FD_CLR(fd, &select_obj->read_fds);
    FD_CLR(fd, &select_obj->write_fds);
    FD_CLR(fd, &select_obj->except_fds);

    // 更新 max_fd
    if (fd == select_obj->max_fd) {
        for (int i = fd - 1; i >= 0; --i) {
            if (FD_ISSET(i, &select_obj->read_fds) ||
                FD_ISSET(i, &select_obj->write_fds) ||
                FD_ISSET(i, &select_obj->except_fds)) {
                select_obj->max_fd = i;
                break;
            }
        }
    }

    return 0;
}

// 设置 select 的超时时间
int nv_select_set_timeout(nv_select_t *select_obj, long sec, long usec) {
    if (!select_obj) {
        return -1;
    }

    select_obj->timeout.tv_sec = sec;
    select_obj->timeout.tv_usec = usec;

    return 0;
}

// 执行 select 操作
int nv_select_wait(nv_select_t *select_obj) {
    if (!select_obj) {
        return -1;
    }

    // 复制文件描述符集合，因为 select 会修改它们
    fd_set read_fds_copy = select_obj->read_fds;
    fd_set write_fds_copy = select_obj->write_fds;
    fd_set except_fds_copy = select_obj->except_fds;

    int retval = select(select_obj->max_fd + 1, &read_fds_copy, &write_fds_copy, &except_fds_copy, &select_obj->timeout);
    if (retval == -1) {
        perror("NV: select failed");
        return -1;
    }

    // 更新 select 对象的文件描述符集合
    select_obj->read_fds = read_fds_copy;
    select_obj->write_fds = write_fds_copy;
    select_obj->except_fds = except_fds_copy;

    return retval;
}

// 销毁 select 结构
void nv_select_destroy(nv_select_t *select_obj) {
    if (select_obj) {
        free(select_obj);
    }
}






int nv_select_main() {
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

    // 初始化 select 结构
    nv_select_t *select_obj = nv_select_init();
    if (!select_obj) {
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 将监听 socket 添加到 select 的集合
    nv_select_add_fd(select_obj, listen_fd, 1, 0, 0);

    for (;;) {
        // 执行 select 操作
        int retval = nv_select_wait(select_obj);
        if (retval == -1) {
            perror("select");
            nv_select_destroy(select_obj);
            close(listen_fd);
            return EXIT_FAILURE;
        }

        // 检查监听 socket 是否有新连接
        if (FD_ISSET(listen_fd, &select_obj->read_fds)) {
            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
            if (conn_fd == -1) {
                perror("accept");
            } else {
                printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // 将新的连接 socket 添加到 select 的集合
                nv_select_add_fd(select_obj, conn_fd, 1, 0, 0);
            }
        }

        // 检查其他 socket 是否有数据可读
        for (int fd = 0; fd <= select_obj->max_fd; ++fd) {
            if (fd == listen_fd || !FD_ISSET(fd, &select_obj->read_fds)) {
                continue;
            }

            char buffer[256];
            ssize_t count = read(fd, buffer, sizeof(buffer));
            if (count == -1) {
                perror("read");
                nv_select_remove_fd(select_obj, fd);
                close(fd);
            } else if (count == 0) {
                // 对方关闭了连接
                nv_select_remove_fd(select_obj, fd);
                close(fd);
            } else {
                // 处理接收到的数据
                buffer[count] = '\0';
                printf("Received: %s\n", buffer);
            }
        }
    }

    // 销毁 select 结构
    nv_select_destroy(select_obj);
    close(listen_fd);

    return EXIT_SUCCESS;
}
