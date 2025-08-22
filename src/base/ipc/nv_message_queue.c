#include "nv_message_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

message_queue_t* nv_init_message_queue(const char* name, long maxmsg, long msgsize, int flags) {
    message_queue_t* queue = (message_queue_t*)malloc(sizeof(message_queue_t));
    if (!queue) {
        fprintf(stderr, "NV: malloc message_queue failed\n");
        return NULL;
    }

    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = (maxmsg > 0) ? maxmsg : 10;
    attr.mq_msgsize = (msgsize > 0) ? msgsize : MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    snprintf(queue->name, sizeof(queue->name), "%s", name ? name : "/nv_default_mq");
    queue->mq = mq_open(queue->name, O_CREAT | O_RDWR | flags, 0644, &attr);
    if (queue->mq == (mqd_t)-1) {
        perror("NV: mq_open");
        free(queue);
        return NULL;
    }

    return queue;
}

int nv_send_message(message_queue_t* queue, const char* msg) {
    if (!queue || !msg) return -1;
    size_t len = strnlen(msg, MAX_MSG_SIZE);
    if (mq_send(queue->mq, msg, len, 0) == -1) {
        perror("NV: mq_send");
        return -1;
    }
    return 0;
}

int nv_receive_message(message_queue_t* queue, char* buffer, size_t buffer_size) {
    if (!queue || !buffer) return -1;
    unsigned int priority = 0;
    ssize_t n = mq_receive(queue->mq, buffer, buffer_size, &priority);
    if (n == -1) {
        perror("NV: mq_receive");
        return -1;
    }
    if ((size_t)n < buffer_size) buffer[n] = '\0';
    return 0;
}

int nv_send_message_timeout(message_queue_t* queue, const char* msg, const struct timespec* abstime) {
    if (!queue || !msg) return -1;
#if defined(_GNU_SOURCE)
    size_t len = strnlen(msg, MAX_MSG_SIZE);
    if (mq_timedsend(queue->mq, msg, len, 0, abstime) == -1) {
        if (errno != ETIMEDOUT) perror("NV: mq_timedsend");
        return -1;
    }
    return 0;
#else
    (void)abstime; return nv_send_message(queue, msg);
#endif
}

int nv_receive_message_timeout(message_queue_t* queue, char* buffer, size_t buffer_size, unsigned int* prio, const struct timespec* abstime) {
    if (!queue || !buffer) return -1;
#if defined(_GNU_SOURCE)
    ssize_t n = mq_timedreceive(queue->mq, buffer, buffer_size, prio, abstime);
    if (n == -1) {
        if (errno != ETIMEDOUT) perror("NV: mq_timedreceive");
        return -1;
    }
    if ((size_t)n < buffer_size) buffer[n] = '\0';
    return 0;
#else
    (void)abstime; (void)prio; return nv_receive_message(queue, buffer, buffer_size);
#endif
}

void nv_destroy_message_queue(message_queue_t* queue) {
    if (!queue) return;
    if (mq_close(queue->mq) == -1) {
        perror("NV: mq_close");
    }
    if (queue->name[0] && mq_unlink(queue->name) == -1) {
        if (errno != ENOENT) perror("NV: mq_unlink");
    }
    free(queue);
}

int message_queue_main() {
    message_queue_t* queue = nv_init_message_queue("/my_queue", 10, MAX_MSG_SIZE, 0);
    if (!queue) return EXIT_FAILURE;

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
