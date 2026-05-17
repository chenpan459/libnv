#include "nv_core_lock.h"

#include <nv_log.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int nv_core_instance_lock_acquire(const char *lock_name, int *lock_fd)
{
    struct sockaddr_un addr;
    int              fd;
    const char      *name;

    if (!lock_fd) {
        return NV_ERROR;
    }
    *lock_fd = -1;

    name = (lock_name && lock_name[0]) ? lock_name : NV_CORE_DEFAULT_LOCK_NAME;

    fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        nv_log_error("instance lock socket failed: %s", strerror(errno));
        return NV_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 2, "libnv-%s", name);

    if (bind(fd, (struct sockaddr *)&addr,
             (socklen_t)(offsetof(struct sockaddr_un, sun_path) + 1 + strlen(name) + 6)) < 0) {
        if (errno == EADDRINUSE) {
            nv_log_error("another instance holds abstract lock: %s", name);
            close(fd);
            return NV_BUSY;
        }
        nv_log_error("instance lock bind failed: %s", strerror(errno));
        close(fd);
        return NV_ERROR;
    }

    *lock_fd = fd;
    nv_log_info("instance abstract lock acquired: %s", name);
    return NV_OK;
}

void nv_core_instance_lock_release(int lock_fd, const char *lock_name)
{
    (void)lock_name;
    if (lock_fd >= 0) {
        close(lock_fd);
    }
}
