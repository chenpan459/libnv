#include "nv_signal.h"
#include "nv_file.h"
#include "nv_fork.h"
#include "nv_list.h"
#include "nv_lock.h"
#include "nv_rb_tree.h"
#include "nv_string.h"
#include "nv_thread.h"
#include "nv_time.h"


int main() {

    return 0;
}


#if NV_SOCKET_DEBUG
int main() {
    // 运行 TCP 服务器（在单独的终端中执行）
    // run_tcp_server("127.0.0.1", 8080);

    // 运行 TCP 客户端
    run_tcp_client("127.0.0.1", 8080);

    return 0;
}

#endif 
#if NV_RB_TREE_DEBUG
int main() {
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
}
#endif

#if NV_RB_TREE_DEBUG
int main() {
    Node* root = NULL;
    root = insert(root, 7);
    root = insert(root, 6);
    root = insert(root, 5);
    root = insert(root, 4);
    root = insert(root, 3);
    root = insert(root, 2);
    root = insert(root, 1);

    printf("中序遍历结果: ");
    inorder(root);

    int num = 4;
    Node* res = search(root, num);
    if (res != NULL)
        printf("\n元素 %d 在树中\n", num);
    else
        printf("\n元素 %d 不在树中\n", num);

    return 0;
}

#endif


#if NV_SIGNAL_DEBUG

int main() {
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

    return 0;
}
#endif

#if FILE_DEBUG
int main() {
    const char* filename = "example.txt";
    FILE* file = nv_open_file(filename, "w");
    if (file != NULL) {
        const char* text = "Hello, World!";
        nv_write_file(text, sizeof(char), 13, file);
        nv_close_file(file);
    }

    file = nv_open_file(filename, "r");
    if (file != NULL) {
        char buffer[100];
        nv_read_file(buffer, sizeof(char), 13, file);
        buffer[13] = '\0'; // 确保字符串以空字符结尾
        printf("读取到的内容: %s\n", buffer);
        nv_close_file(file);
    }

    return 0;
}
#endif


#if NV_FORK_DEBUG

int main() {
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

    return 0;
}

// 示例子进程函数
void child_process() {
    printf("子进程正在执行...\n");
    sleep(2); // 模拟子进程工作
    printf("子进程执行完毕。\n");
}
int main() {
    pid_t pid = nv_create_process(child_process);
    if (pid > 0) {
        printf("父进程等待子进程结束...\n");
        int exit_status = nv_wait_process(pid);
        printf("子进程结束，退出状态 = %d\n", exit_status);
    }

    return 0;
}
#endif



#if DEBUG_LIST
int main() {
    Node* head = NULL;

    head = insertEnd(head, 1);
    head = insertEnd(head, 2);
    head = insertEnd(head, 3);
    head = insertEnd(head, 4);

    printf("链表内容: ");
    printList(head);

    head = deleteNode(head, 3);
    printf("删除节点 3 后的链表: ");
    printList(head);

    return 0;
}
#endif




#if NV_LOCK_DEBUG
// 示例：使用互斥锁保护共享资源
void critical_section() {
    // 临界区代码
    printf("进入临界区...\n");
    sleep(1); // 模拟耗时操作
    printf("离开临界区。\n");
}

int main() {
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

    return 0;
}

#endif