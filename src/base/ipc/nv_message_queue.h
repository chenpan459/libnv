#ifndef _NV_MESSAGE_QUEUE_H_INCLUDED_
#define _NV_MESSAGE_QUEUE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>
#include <mqueue.h>
#include <time.h>

#define MAX_MSG_SIZE 256

typedef struct {
    mqd_t mq;
    char  name[64];
} message_queue_t;

message_queue_t* nv_init_message_queue(const char* name, long maxmsg, long msgsize, int flags);
int nv_send_message(message_queue_t* queue, const char* msg);
int nv_receive_message(message_queue_t* queue, char* buffer, size_t buffer_size);

/* 新增：非阻塞与超时接口 */
int nv_send_message_timeout(message_queue_t* queue, const char* msg, const struct timespec* abstime);
int nv_receive_message_timeout(message_queue_t* queue, char* buffer, size_t buffer_size, unsigned int* prio, const struct timespec* abstime);

void nv_destroy_message_queue(message_queue_t* queue);

int message_queue_main();

#ifdef __cplusplus
}
#endif

#endif 