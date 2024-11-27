#ifndef _NV_HEAP_H_INCLUDED_
#define _NV_HEAP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include  <nv_config.h>

#include <stdbool.h>

#include <stdbool.h>



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 10

// 堆结构体
typedef struct {
    int* data;
    int size;
    int capacity;
} nv_heap_t;



// 初始化堆
nv_heap_t* nv_heap_init() ;
// 销毁堆
void nv_heap_destroy(nv_heap_t* heap) ;
// 检查堆是否为空
bool nv_heap_is_empty(nv_heap_t* heap) ;
// 获取堆顶元素
int nv_heap_peek(nv_heap_t* heap) ;

// 上滤操作
void nv_heap_sift_up(nv_heap_t* heap, int index) ;

// 下滤操作
void nv_heap_sift_down(nv_heap_t* heap, int index) ;
// 插入元素
void nv_heap_insert(nv_heap_t* heap, int value) ;

// 删除堆顶元素
int nv_heap_remove(nv_heap_t* heap) ;


int nv_heap_main() ;

    
#ifdef __cplusplus
}
#endif

#endif 