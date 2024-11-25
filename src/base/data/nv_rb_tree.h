
#ifndef _NV_RB_TREE_H_INCLUDED_
#define _NV_RB_TREE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"

// 定义颜色
#define RED   0
#define BLACK 1

// 定义节点结构
typedef struct Node {
    int data;
    int color;
    struct Node *left, *right, *parent;
} nv_rb_Node;

nv_rb_Node* nv_rb_newNode(int data) ;
nv_rb_Node* nv_rb_leftRotate(nv_rb_Node* root, nv_rb_Node* pt) ;
nv_rb_Node* nv_rb_rightRotate(nv_rb_Node* root, nv_rb_Node* pt) ;
nv_rb_Node* nv_rb_insertFix(nv_rb_Node* root, nv_rb_Node* pt) ;
nv_rb_Node* nv_rb_insertBST(nv_rb_Node* root, nv_rb_Node* pt) ;
nv_rb_Node* nv_rb_insert(nv_rb_Node* root, int data) ;
nv_rb_Node* nv_rb_search(nv_rb_Node* root, int data) ;
void nv_rb_inorder(nv_rb_Node* root) ;

int nv_rb_main();

#ifdef __cplusplus
}
#endif

#endif
