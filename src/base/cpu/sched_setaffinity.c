/*************
 * 在多核处理器上，将进程绑定到特定的 CPU 核可以提高性能，减少缓存失效和上下文切换的开销。Nginx 在某些情况下可能会使用这种技术来优化性能。以下是一些关键点和实现方法：

1. 为什么绑定进程到 CPU 核？
缓存局部性：将进程绑定到特定的 CPU 核可以提高缓存命中率，因为数据会保留在该核的缓存中。
减少上下文切换：绑定进程可以减少操作系统在不同 CPU 核之间切换进程的开销。
提高预测性：绑定进程可以提高 CPU 的分支预测和指令流水线的效率。
2. 实现方法
在 Linux 系统中，可以使用 sched_setaffinity 系统调用来绑定进程到特定的 CPU 核。以下是一个简单的示例代码：
****/

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

void bind_process_to_cpu(int cpu) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}
/*
int main() {
    int cpu = 0; // 绑定到 CPU 0
    bind_process_to_cpu(cpu);

    // 进程的主要逻辑
    while (1) {
        // 执行任务
        sleep(1);
    }

    return 0;
}*/