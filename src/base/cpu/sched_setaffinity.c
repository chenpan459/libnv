/*************
 * 在多核处理器上，将进程绑定到特定的 CPU 核可以提高性能，减少缓存失效和上下文切换的开销。Nginx 在某些情况下可能会使用这种技术来优化性能。以下是一些关键点和实现方法：

1. 为什么绑定进程到 CPU 核？
缓存局部性：将进程绑定到特定的 CPU 核可以提高缓存命中率，因为数据会保留在该核的缓存中。
减少上下文切换：绑定进程可以减少操作系统在不同 CPU 核之间切换进程的开销。
提高预测性：绑定进程可以提高 CPU 的分支预测和指令流水线的效率。
2. 实现方法
在 Linux 系统中，可以使用 sched_setaffinity 系统调用来绑定进程到特定的 CPU 核。以下是一个简单的示例代码：
****/


/**
 * 
 * 
项目	sched_setaffinity	       cpuset_setaffinity	      SetProcessAffinityMask
平台	       Linux	            Linux (特定系统/框架)	       Windows
控制对象	进程/线程	               进程/线程/容器资源限制	          进程
掩码类型	cpu_set_t	              通常基于 cpuset 文件系统	    位掩码（DWORD_PTR）
使用场景	线程/进程绑核，高性能调度	嵌入式/容器 CPU 资源控制	   Windows 应用绑核
控制粒度	线程/进程	               系统定制，可能支持容器级	       进程
是否标准	是（glibc/内核）	       否，依赖具体实现	              是（Win32 API）
✅ 推荐使用建议：
使用平台	                建议接口
Linux	                 使用 sched_setaffinity() 是最标准、最通用的做法
Windows	                 使用 SetProcessAffinityMask()
嵌入式 Linux / 特定厂商   SDK	使用 cpuset_setaffinity() 或其封装接口，需看平台支持

*/
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>


void bind_process_to_cpu(int cpu) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}
