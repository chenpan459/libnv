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
