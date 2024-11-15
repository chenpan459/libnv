#ifndef _NV_MESSAGE_QUEUE_H_INCLUDED_
#define _NV_MESSAGE_QUEUE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_util_include.h"


#define QUEUE_NAME "/my_queue"
#define MAX_MSG_SIZE 256

typedef struct {
    mqd_t mq;
} message_queue_t;

// NV封装的性能监控宏
#define NV_PERF_START() clock_t start_time = clock();
#define NV_PERF_END(func_name) do { \
    clock_t end_time = clock(); \
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC; \
    printf("NV: %s took %.2f seconds.\n", func_name, time_spent); \
} while(0)

// NV封装的错误处理宏
#define NV_ERROR_HANDLE(error_msg) do { \
    perror(error_msg); \
    return -1; \
} while(0)



message_queue_t* nv_init_message_queue();
int nv_send_message(message_queue_t* queue, const char* msg);
int nv_receive_message(message_queue_t* queue, char* buffer, size_t buffer_size);
void nv_destroy_message_queue(message_queue_t* queue) ;



int message_queue_main() ;

#ifdef __cplusplus
}
#endif

#endif 