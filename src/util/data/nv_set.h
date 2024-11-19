#ifndef _NV_SET_H_INCLUDED_
#define _NV_SET_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_util_include.h"

#include <stdbool.h>
// 集合节点结构体
typedef struct nv_set_node {
    char* data;
    struct nv_set_node* next;
} nv_set_node_t;

// 集合结构体
typedef struct {
    nv_set_node_t* head;
    int size;
} nv_set_t;

// 初始化集合
nv_set_t* nv_set_init() ;
// 销毁集合
void nv_set_destroy(nv_set_t* set) ;

// 检查元素是否存在
bool nv_set_contains(nv_set_t* set, const char* data) ;

// 添加元素
void nv_set_add(nv_set_t* set, const char* data) ;
// 删除元素
void nv_set_remove(nv_set_t* set, const char* data) ;

// 获取集合大小
int nv_set_size(nv_set_t* set) ;

int nv_set_main() ;


#ifdef __cplusplus
}
#endif

#endif 