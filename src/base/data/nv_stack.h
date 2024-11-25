#ifndef _NV_STACK_H_INCLUDED_
#define _NV_STACK_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_base_include.h"


#include <stdbool.h>

// 栈节点结构体
typedef struct nv_stack_node {
    void* data;
    struct nv_stack_node* next;
} nv_stack_node_t;

// 栈结构体
typedef struct {
    nv_stack_node_t* top;
} nv_stack_t;

// 初始化栈
nv_stack_t* nv_stack_init() ;
// 销毁栈
void nv_stack_destroy(nv_stack_t* stack) ;
// 压栈
void nv_stack_push(nv_stack_t* stack, void* data) ;
// 弹栈
void* nv_stack_pop(nv_stack_t* stack) ;
// 检查栈是否为空
bool nv_stack_is_empty(nv_stack_t* stack) ;


int nv_stack_main() ;

#ifdef __cplusplus
}
#endif

#endif 