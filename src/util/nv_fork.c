#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "nv_fork.h"

// 定义函数指针类型
typedef void (*process_func_t)(void);

// 创建并执行子进程
pid_t nv_create_process(process_func_t func) {
    pid_t pid = fork();

    if (pid < 0) {
        // fork 失败
        perror("nv_create_process: fork 失败");
        return -1;
    } else if (pid == 0) {
        // 子进程
        func();
        exit(0); // 子进程执行完毕后退出
    }

    // 父进程返回子进程的 PID
    return pid;
}

// 等待子进程结束
int nv_wait_process(pid_t pid) {
    int status;
    if (waitpid(pid, &status, 0) != -1) {
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return -1;
}





int nv_create_daemon() {
    // 1. 创建子进程，父进程退出
    pid_t pid = fork();
    if (pid < 0) {
        perror("nv_create_daemon: fork 失败");
        return -1;
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }

    // 2. 脱离当前会话
    if (setsid() < 0) {
        perror("nv_create_daemon: setsid 失败");
        return -1;
    }

    // 3. 改变工作目录到根目录
    if (chdir("/") < 0) {
        perror("nv_create_daemon: chdir 失败");
        return -1;
    }

    // 4. 重设文件模式创建掩码
    umask(0);

    // 5. 关闭所有打开的文件描述符
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; --fd) {
        close(fd);
    }

    // 6. 打开标准输入、输出和错误输出到 /dev/null
    open("/dev/null", O_RDONLY);  // 标准输入
    open("/dev/null", O_RDWR);   // 标准输出
    open("/dev/null", O_RDWR);   // 标准错误输出

    return 0;
}



