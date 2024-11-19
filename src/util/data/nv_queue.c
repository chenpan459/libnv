#include "nv_queue.h"

/*********************************************
 * 要封装一个队列（Queue）数据结构，可以定义一个队列结构体，并提供基本的操作函数，
 * 如入队（enqueue）、出队（dequeue）和检查队列是否为空。以下是一个示例：

计划
定义队列节点和队列结构体。
实现初始化队列、销毁队列、入队、出队和检查队列是否为空的函数。
*********************************************************/
// 初始化队列
nv_queue_t* nv_queue_init() {
    nv_queue_t* queue = (nv_queue_t*)malloc(sizeof(nv_queue_t));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

// 销毁队列
void nv_queue_destroy(nv_queue_t* queue) {
    nv_queue_node_t* current = queue->front;
    while (current) {
        nv_queue_node_t* temp = current;
        current = current->next;
        free(temp);
    }
    free(queue);
}

// 入队
void nv_queue_enqueue(nv_queue_t* queue, void* data) {
    nv_queue_node_t* new_node = (nv_queue_node_t*)malloc(sizeof(nv_queue_node_t));
    new_node->data = data;
    new_node->next = NULL;
    if (queue->rear) {
        queue->rear->next = new_node;
    } else {
        queue->front = new_node;
    }
    queue->rear = new_node;
}

// 出队
void* nv_queue_dequeue(nv_queue_t* queue) {
    if (queue->front == NULL) {
        return NULL;  // 队列为空
    }
    nv_queue_node_t* temp = queue->front;
    void* data = temp->data;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return data;
}

// 检查队列是否为空
bool nv_queue_is_empty(nv_queue_t* queue) {
    return queue->front == NULL;
}

int nv_queue_main() {
    nv_queue_t* queue = nv_queue_init();

    int data1 = 1, data2 = 2, data3 = 3;
    nv_queue_enqueue(queue, &data1);
    nv_queue_enqueue(queue, &data2);
    nv_queue_enqueue(queue, &data3);

    while (!nv_queue_is_empty(queue)) {
        int* data = (int*)nv_queue_dequeue(queue);
        printf("Dequeued: %d\n", *data);
    }

    nv_queue_destroy(queue);
    return 0;
}