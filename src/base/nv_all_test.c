#include "ipc/nv_signal.h"
#include "sys/nv_file.h"
#include "sys/nv_fork.h"
#include "data/nv_list.h"
#include "data/nv_rb_tree.h"
#include "sys/nv_string.h"
#include "sys/nv_thread.h"
#include "sys/nv_time.h"
#include "algo/nv_md5.h"
#include "data/nv_hash_table.h"
#include "net/nv_socket.h"
#include "sys/nv_thread.h"
#include "ipc/nv_signal.h"
#include "sys/nv_lock.h"
#include "algo/nv_base64.h"
#include "sys/nv_thread_pool.h"
#include "ipc/nv_message_queue.h"
#include "ipc/nv_fifo.h"
#include "ipc/nv_pipe.h"
#include "ipc/nv_semaphore.h"
#include "ipc/nv_shm.h"
#include "net/nv_socket_epoll.h"
#include "net/nv_socket_poll.h"
#include "net/nv_socket_select.h"
#include "sys/nv_timer_task.h"
#include "ipc/nv_mmap.h"
#include "net/nv_unix_socket.h"
#include "log/nv_log.h"



#define CALCULATE_MD5   0
#define NV_HASH_TABLE   0
#define NV_SOCKET_DEBUG 0
#define NV_RB_THREAD_DEBUG 0
#define NV_RB_TREE_DEBUG  0
#define NV_SIGNAL_DEBUG 0
#define NV_FILE_DEBUG 0
#define NV_FORK_DEBUG 0
#define NV_DEBUG_LIST 0
#define NV_LOCK_DEBUG 0

// 示例线程函数
void* thread_function(void* arg) {
    int thread_num = *((int*)arg);
    printf("线程 %d 正在执行...\n", thread_num);
    sleep(1); // 模拟线程工作
    printf("线程 %d 执行完毕。\n", thread_num);
    return NULL;
}



// 示例子进程函数
void child_process() {
    printf("子进程正在执行...\n");
    sleep(2); // 模拟子进程工作
    printf("子进程执行完毕。\n");
}

// 示例：使用互斥锁保护共享资源
void critical_section() {
    // 临界区代码
    printf("进入临界区...\n");
    sleep(1); // 模拟耗时操作
    printf("离开临界区。\n");
}

int main() {
    nv_log_debug("log new\n");

    //nv_time_main();    
    //nv_base64_main();
    //nv_thread_pool_main();    
    //message_queue_main();
    //nv_fifo_main();
    //nv_pipe_main() ;    
    //nv_semaphore_main();
    //nv_shm_main() ;
    //nv_epoll_main() ;
  //  nv_poll_main() ;
 
     //nv_md5_main();
//    nv_hash_table_main();
//   nv_socket_main();
//   nv_rb_main();
  // nv_file_main();
///   nv_list_main();

 // nv_timer_task_main() ;
 //nv_unix_socket_server_main();
//nv_unix_socket_client_main();
 


#if NV_RB_THREAD_DEBUG

    pthread_t threads[5];
    int thread_args[5];
    pthread_attr_t attr;

    // 初始化线程属性
    if (nv_init_thread_attr(&attr, PTHREAD_CREATE_JOINABLE, 0) != 0) {
        perror("nv_init_thread_attr 失败");
        return 1;
    }

    // 创建 5 个线程
    for (int i = 0; i < 5; ++i) {
        thread_args[i] = i + 1;
        if (nv_create_thread(&threads[i], &attr, thread_function, &thread_args[i]) != 0) {
            perror("nv_create_thread 失败");
            return 1;
        }
    }

    // 销毁线程属性对象
    pthread_attr_destroy(&attr);

    // 等待所有线程结束
    for (int i = 0; i < 5; ++i) {
        if (nv_join_thread(threads[i]) != 0) {
            perror("nv_join_thread 失败");
            return 1;
        }
    }

    printf("所有线程已结束。\n");
    return 0;

#endif


#if NV_SIGNAL_DEBUG

    pid_t pid = getpid();

    // 注册信号处理函数
    if (nv_signal_register(SIGUSR1, signal_handler) != 0) {
        perror("nv_signal_register 失败");
        return 1;
    }

    printf("进程 ID: %d\n", pid);
    printf("等待信号 SIGUSR1...\n");

    // 暂停等待信号
    pause();

    // 发送信号给自己
    printf("发送信号 SIGUSR1 给自己\n");
    if (nv_signal_send(pid, SIGUSR1) != 0) {
        perror("nv_signal_send 失败");
        return 1;
    }

    // 再次暂停等待信号
    pause();


#endif



#if NV_FORK_DEBUG


    // 创建守护进程
    if (nv_create_daemon() < 0) {
        perror("创建守护进程失败");
        return 1;
    }

    // 守护进程的核心工作
    while (1) {
        // ... 守护进程的工作代码 ...
        sleep(1); // 示例：每隔 1 秒执行一次
    }


    pid_t pid = nv_create_process(child_process);
    if (pid > 0) {
        printf("父进程等待子进程结束...\n");
        int exit_status = nv_wait_process(pid);
        printf("子进程结束，退出状态 = %d\n", exit_status);
    }
#endif






#if NV_LOCK_DEBUG
    nv_mutex_t mutex;
    
    // 初始化互斥锁
    if (nv_mutex_init(&mutex) != 0) {
        perror("nv_mutex_init 失败");
        return 1;
    }

    // 创建线程
    pthread_t threads[2];
    for (int i = 0; i < 2; ++i) {
        if (pthread_create(&threads[i], NULL, (void* (*)(void*))critical_section, NULL) != 0) {
            perror("pthread_create 失败");
            return 1;
        }
    }

    // 等待线程结束
    for (int i = 0; i < 2; ++i) {
        pthread_join(threads[i], NULL);
    }

    // 销毁互斥锁
    nv_mutex_destroy(&mutex);


#endif
    return 0;

    
}



