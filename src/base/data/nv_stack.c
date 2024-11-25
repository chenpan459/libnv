#include "nv_stack.h"

/*要封装一个栈（Stack）数据结构，可以定义一个栈结构体，并提供基本的操作函数，如压栈（push）、弹栈（pop）和检查栈是否为空。以下是一个示例：

计划
定义栈节点和栈结构体。
实现初始化栈、销毁栈、压栈、弹栈和检查栈是否为空的函数。
*/
// 初始化栈
nv_stack_t* nv_stack_init() {
    nv_stack_t* stack = (nv_stack_t*)malloc(sizeof(nv_stack_t));
    stack->top = NULL;
    return stack;
}

// 销毁栈
void nv_stack_destroy(nv_stack_t* stack) {
    nv_stack_node_t* current = stack->top;
    while (current) {
        nv_stack_node_t* temp = current;
        current = current->next;
        free(temp);
    }
    free(stack);
}

// 压栈
void nv_stack_push(nv_stack_t* stack, void* data) {
    nv_stack_node_t* new_node = (nv_stack_node_t*)malloc(sizeof(nv_stack_node_t));
    new_node->data = data;
    new_node->next = stack->top;
    stack->top = new_node;
}

// 弹栈
void* nv_stack_pop(nv_stack_t* stack) {
    if (stack->top == NULL) {
        return NULL;  // 栈为空
    }
    nv_stack_node_t* temp = stack->top;
    void* data = temp->data;
    stack->top = stack->top->next;
    free(temp);
    return data;
}

// 检查栈是否为空
bool nv_stack_is_empty(nv_stack_t* stack) {
    return stack->top == NULL;
}

int nv_stack_main() {
    nv_stack_t* stack = nv_stack_init();

    int data1 = 1, data2 = 2, data3 = 3;
    nv_stack_push(stack, &data1);
    nv_stack_push(stack, &data2);
    nv_stack_push(stack, &data3);

    while (!nv_stack_is_empty(stack)) {
        int* data = (int*)nv_stack_pop(stack);
        printf("Popped: %d\n", *data);
    }

    nv_stack_destroy(stack);
    return 0;
}