#include "nv_message_queue.h"



// 初始化消息队列
// 初始化消息队列的函数
message_queue_t* nv_init_message_queue() {
    // 开始性能监控
    NV_PERF_START();
    // 动态分配内存用于存储消息队列结构体
    message_queue_t* queue = (message_queue_t*)malloc(sizeof(message_queue_t));
    // 检查内存分配是否成功
    if (!queue) {
        // 如果内存分配失败，打印错误信息
        printf("Failed to allocate memory for message queue");
        // 返回NULL表示初始化失败
        return NULL;
    }

    // 定义消息队列属性结构体
    struct mq_attr attr;
    // 设置消息队列标志位为0，表示无特殊属性
    attr.mq_flags = 0;
    // 设置消息队列中最大消息数为10
    attr.mq_maxmsg = 10;
    // 设置单个消息的最大大小为MAX_MSG_SIZE
    attr.mq_msgsize = MAX_MSG_SIZE;
    // 设置当前消息队列为空，即当前消息数为0
    attr.mq_curmsgs = 0;

    // 打开或创建消息队列，返回消息队列描述符
    queue->mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    // 检查消息队列是否成功打开或创建
    if (queue->mq == (mqd_t)-1) {
        // 如果失败，打印错误信息
        perror("Failed to open message queue");
        // 释放之前分配的内存
        free(queue);
        // 返回NULL表示初始化失败
        return NULL;
    }

    // 结束性能监控，并记录性能数据
    NV_PERF_END("nv_init_message_queue");
    // 返回初始化成功的消息队列指针
    return queue;
}


// 发送消息到队列
int nv_send_message(message_queue_t* queue, const char* msg) {
    NV_PERF_START();
    if (mq_send(queue->mq, msg, strlen(msg), 0) == -1) {
        NV_ERROR_HANDLE("Failed to send message");
    }
    NV_PERF_END("nv_send_message");

    return 0;
}

// 从队列接收消息
int nv_receive_message(message_queue_t* queue, char* buffer, size_t buffer_size) {
    NV_PERF_START();
    unsigned int priority;
    if (mq_receive(queue->mq, buffer, buffer_size, &priority) == -1) {
        NV_ERROR_HANDLE("Failed to receive message");
    }
    NV_PERF_END("nv_receive_message");

    return 0;
}

// 销毁消息队列
void nv_destroy_message_queue(message_queue_t* queue) {
    NV_PERF_START();
    if (mq_close(queue->mq) == -1) {
        perror("Failed to close message queue");
    }
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("Failed to unlink message queue");
    }
    free(queue);
    NV_PERF_END("nv_destroy_message_queue");
}

// 定义一个函数，用于演示消息队列的使用
int message_queue_main() {
    // 初始化消息队列，返回一个指向消息队列的指针
    message_queue_t* queue = nv_init_message_queue();
    // 检查消息队列是否成功初始化
    if (!queue) {
        // 如果初始化失败，返回失败状态码
        return EXIT_FAILURE;
    }

    // 定义一个常量字符串，作为要发送的消息
    const char* msg = "Hello, Message Queue!";
    // 发送消息到消息队列，检查发送是否成功
    if (nv_send_message(queue, msg) == -1) {
        // 如果发送失败，销毁消息队列并返回失败状态码
        nv_destroy_message_queue(queue);
        return EXIT_FAILURE;
    }

    // 定义一个缓冲区，用于接收消息
    char buffer[MAX_MSG_SIZE];
    // 从消息队列接收消息到缓冲区，检查接收是否成功
    if (nv_receive_message(queue, buffer, sizeof(buffer)) == -1) {
        // 如果接收失败，销毁消息队列并返回失败状态码
        nv_destroy_message_queue(queue);
        return EXIT_FAILURE;
    }

    // 打印接收到的消息
    printf("Received message: %s\n", buffer);

    // 销毁消息队列，释放资源
    nv_destroy_message_queue(queue);
    // 返回成功状态码
    return EXIT_SUCCESS;
}
