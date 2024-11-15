#include "nv_message_queue.h"



// 初始化消息队列
message_queue_t* nv_init_message_queue() {
    NV_PERF_START();
    message_queue_t* queue = (message_queue_t*)malloc(sizeof(message_queue_t));
    if (!queue) {
        printf("Failed to allocate memory for message queue");
        return NULL;
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    queue->mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (queue->mq == (mqd_t)-1) {
        perror("Failed to open message queue");
        free(queue);
        return NULL;
    }

    NV_PERF_END("nv_init_message_queue");
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

int message_queue_main() {
    message_queue_t* queue = nv_init_message_queue();
    if (!queue) {
        return EXIT_FAILURE;
    }

    const char* msg = "Hello, Message Queue!";
    if (nv_send_message(queue, msg) == -1) {
        nv_destroy_message_queue(queue);
        return EXIT_FAILURE;
    }

    char buffer[MAX_MSG_SIZE];
    if (nv_receive_message(queue, buffer, sizeof(buffer)) == -1) {
        nv_destroy_message_queue(queue);
        return EXIT_FAILURE;
    }

    printf("Received message: %s\n", buffer);

    nv_destroy_message_queue(queue);
    return EXIT_SUCCESS;
}
