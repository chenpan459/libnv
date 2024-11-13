#include "signal.h"

// 定义信号处理函数类型
typedef void (*nv_signal_handler_t)(int);

// 注册信号处理函数
int nv_signal_register(int signum, nv_signal_handler_t handler) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return sigaction(signum, &sa, NULL);
}

// 发送信号
int nv_signal_send(pid_t pid, int signum) {
    return kill(pid, signum);
}

// 示例信号处理函数
void signal_handler(int signum) {
    printf("接收到信号 %d\n", signum);
}

