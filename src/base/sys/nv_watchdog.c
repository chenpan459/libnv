#include "nv_watchdog.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef WDIOC_KEEPALIVE
#ifdef __linux__
#include <linux/watchdog.h>
#endif
#endif

static int g_wdt_fd = -1;

int nv_watchdog_open(const char *device)
{
    const char *dev = device;

    if (g_wdt_fd >= 0) {
        return NV_OK;
    }
    if (!dev || !dev[0]) {
        dev = "/dev/watchdog";
    }

    g_wdt_fd = open(dev, O_WRONLY | O_CLOEXEC);
    if (g_wdt_fd < 0) {
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_watchdog_feed(void)
{
    int rc = 0;

    if (g_wdt_fd < 0) {
        return NV_DECLINED;
    }

#ifdef WDIOC_KEEPALIVE
    rc = ioctl(g_wdt_fd, WDIOC_KEEPALIVE, 0);
    if (rc == 0) {
        return NV_OK;
    }
#endif
    if (write(g_wdt_fd, "\0", 1) == 1) {
        return NV_OK;
    }
    return NV_ERROR;
}

void nv_watchdog_close(void)
{
    if (g_wdt_fd >= 0) {
        write(g_wdt_fd, "V", 1);
        close(g_wdt_fd);
        g_wdt_fd = -1;
    }
}

int nv_watchdog_feed_cmd(const char *cmd)
{
    if (!cmd || !cmd[0]) {
        return NV_DECLINED;
    }
    return (system(cmd) == 0) ? NV_OK : NV_ERROR;
}
