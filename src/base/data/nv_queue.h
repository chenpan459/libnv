#ifndef _NV_QUEUW_H_INCLUDED_
#define _NV_QUEUW_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_util_include.h"

#include <stdbool.h>

// 队列节点结构体
typedef struct nv_queue_node {
    void* data;
    struct nv_queue_node* next;
} nv_queue_node_t;

// 队列结构体
typedef struct {
    nv_queue_node_t* front;
    nv_queue_node_t* rear;
} nv_queue_t;


// 初始化队列
nv_queue_t* nv_queue_init() ;
// 销毁队列
void nv_queue_destroy(nv_queue_t* queue) ;
// 入队
void nv_queue_enqueue(nv_queue_t* queue, void* data) ;
// 出队
void* nv_queue_dequeue(nv_queue_t* queue) ;
// 检查队列是否为空
bool nv_queue_is_empty(nv_queue_t* queue) ;



int nv_queue_main() ;


#ifdef __cplusplus
}
#endif

#endif 