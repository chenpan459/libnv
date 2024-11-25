#ifndef _NV_DICT_H_INCLUDED_
#define _NV_DICT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_base_include.h"

#include <stdbool.h>

#define TABLE_SIZE 100

// 字典节点结构体
typedef struct nv_dict_node {
    char* key;
    void* value;
    struct nv_dict_node* next;
} nv_dict_node_t;

// 字典结构体
typedef struct {
    nv_dict_node_t** buckets;
} nv_dict_t;



// 初始化字典
nv_dict_t* nv_dict_init() ;
// 销毁字典
void nv_dict_destroy(nv_dict_t* dict) ;

// 插入键值对
void nv_dict_insert(nv_dict_t* dict, const char* key, void* value) ;

// 查找值
void* nv_dict_lookup(nv_dict_t* dict, const char* key) ;

// 删除键值对
void nv_dict_remove(nv_dict_t* dict, const char* key) ;
// 检查字典是否为空
bool nv_dict_is_empty(nv_dict_t* dict) ;




int nv_dict_main() ;

    
#ifdef __cplusplus
}
#endif

#endif 