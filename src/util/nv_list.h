#ifndef _NV_LIST_H_INCLUDED_
#define _NV_LIST_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_util_include.h"
// 定义节点结构
typedef struct nv_list_Node {
    int data;
    struct nv_list_Node* prev;
    struct nv_list_Node* next;
} nv_list_Node;


void nv_list_printList(nv_list_Node* head) ;
nv_list_Node* nv_list_deleteNode(nv_list_Node* head, int key) ;
nv_list_Node* nv_list_insertEnd(nv_list_Node* head, int data) ;
nv_list_Node* nv_list_createNode(int data) ;
int nv_list_main();

#ifdef __cplusplus
}
#endif

#endif 