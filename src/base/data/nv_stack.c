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
// 定义一个函数，用于将数据压入栈中
void nv_stack_push(nv_stack_t* stack, void* data) {
    // 分配内存空间，用于存储新的栈节点
    nv_stack_node_t* new_node = (nv_stack_node_t*)malloc(sizeof(nv_stack_node_t));
    // 将传入的数据赋值给新节点的数据域
    new_node->data = data;
    // 将新节点的下一个节点指向当前栈的顶部节点
    new_node->next = stack->top;
    // 更新栈的顶部节点为新节点
    stack->top = new_node;
}

// 弹栈
// 函数声明：nv_stack_pop
// 功能：从栈中弹出栈顶元素
// 参数：nv_stack_t* stack - 指向栈的指针
// 返回值：void* - 弹出的栈顶元素的指针，如果栈为空则返回NULL
void* nv_stack_pop(nv_stack_t* stack) {
    // 检查栈是否为空
    if (stack->top == NULL) {
        return NULL;  // 栈为空，返回NULL
    }
    // 保存当前栈顶节点
    nv_stack_node_t* temp = stack->top;
    // 获取栈顶节点的数据
    void* data = temp->data;
    // 更新栈顶指针，指向下一个节点
    stack->top = stack->top->next;
    // 释放原栈顶节点的内存
    free(temp);
    // 返回栈顶节点的数据
    return data;
}

// 检查栈是否为空
// 定义一个函数，用于检查给定的栈是否为空
// 参数：stack - 指向nv_stack_t类型的指针，表示栈的结构体
// 返回值：布尔值，如果栈为空则返回true，否则返回false
bool nv_stack_is_empty(nv_stack_t* stack) {
    // 检查栈的top指针是否为NULL
    // 如果top为NULL，表示栈中没有元素，即栈为空
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