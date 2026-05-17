#include "nv_signal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signalfd.h>

#define NV_SIGNAL_SLOT_MAX  32

typedef struct {
    int                   used;
    int                   signum;
    nv_signal_handler_t   handler;
} nv_signal_slot_t;

static sigset_t         g_mask;
static int              g_signalfd = -1;
static nv_signal_slot_t g_slots[NV_SIGNAL_SLOT_MAX];
static int              g_inited;

static int nv_signal_update_fd(void)
{
    int nfd;

    if (g_signalfd < 0) {
        nfd = signalfd(-1, &g_mask, SFD_CLOEXEC | SFD_NONBLOCK);
    } else {
        nfd = signalfd(g_signalfd, &g_mask, SFD_CLOEXEC | SFD_NONBLOCK);
    }
    if (nfd < 0) {
        return -1;
    }
    if (g_signalfd >= 0 && nfd != g_signalfd) {
        close(g_signalfd);
    }
    g_signalfd = nfd;
    return 0;
}

static nv_signal_handler_t nv_signal_find_handler(int signum)
{
    int i;

    for (i = 0; i < NV_SIGNAL_SLOT_MAX; i++) {
        if (g_slots[i].used && g_slots[i].signum == signum) {
            return g_slots[i].handler;
        }
    }
    return NULL;
}

int nv_signal_init(void)
{
    if (g_inited) {
        return 0;
    }

    sigemptyset(&g_mask);
    sigprocmask(SIG_BLOCK, &g_mask, NULL);
    memset(g_slots, 0, sizeof(g_slots));

    if (nv_signal_update_fd() != 0) {
        return -1;
    }

    g_inited = 1;
    return 0;
}

void nv_signal_shutdown(void)
{
    if (g_signalfd >= 0) {
        close(g_signalfd);
        g_signalfd = -1;
    }
    sigemptyset(&g_mask);
    sigprocmask(SIG_UNBLOCK, &g_mask, NULL);
    memset(g_slots, 0, sizeof(g_slots));
    g_inited = 0;
}

int nv_signal_register(int signum, nv_signal_handler_t handler)
{
    int i;

    if (!handler || signum <= 0 || signum >= NSIG) {
        return -1;
    }
    if (!g_inited && nv_signal_init() != 0) {
        return -1;
    }

    for (i = 0; i < NV_SIGNAL_SLOT_MAX; i++) {
        if (g_slots[i].used && g_slots[i].signum == signum) {
            g_slots[i].handler = handler;
            return 0;
        }
    }

    for (i = 0; i < NV_SIGNAL_SLOT_MAX; i++) {
        if (!g_slots[i].used) {
            g_slots[i].used    = 1;
            g_slots[i].signum  = signum;
            g_slots[i].handler = handler;
            sigaddset(&g_mask, signum);
            sigprocmask(SIG_BLOCK, &g_mask, NULL);
            return nv_signal_update_fd();
        }
    }

    return -1;
}

int nv_signal_fd(void)
{
    return g_signalfd;
}

void nv_signal_dispatch(int signalfd_fd)
{
    struct signalfd_siginfo fdsi;
    nv_signal_handler_t     handler;
    ssize_t                 n;

    if (signalfd_fd < 0) {
        return;
    }

    for (;;) {
        n = read(signalfd_fd, &fdsi, sizeof(fdsi));
        if (n < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                break;
            }
            break;
        }
        if (n != (ssize_t)sizeof(fdsi)) {
            break;
        }

        handler = nv_signal_find_handler((int)fdsi.ssi_signo);
        if (handler) {
            handler((int)fdsi.ssi_signo);
        }
    }
}

int nv_signal_send(pid_t pid, int signum)
{
    return kill(pid, signum);
}

void signal_handler(int signum)
{
    printf("接收到信号 %d\n", signum);
}
