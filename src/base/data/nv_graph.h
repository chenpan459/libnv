#ifndef _NV_GRAPH_H_INCLUDED_
#define _NV_GRAPH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
#include "../nv_base_include.h"

#include <stdbool.h>

#include <stdbool.h>





// 图的边结构体
typedef struct nv_edge {
    int dest;
    struct nv_edge* next;
} nv_edge_t;

// 图的顶点结构体
typedef struct nv_vertex {
    int id;
    nv_edge_t* edges;
    struct nv_vertex* next;
} nv_vertex_t;

// 图结构体
typedef struct {
    nv_vertex_t* vertices;
} nv_graph_t;


// 初始化图
nv_graph_t* nv_graph_init() ;
// 销毁图
void nv_graph_destroy(nv_graph_t* graph) ;

// 添加顶点
void nv_graph_add_vertex(nv_graph_t* graph, int id) ;
// 添加边
void nv_graph_add_edge(nv_graph_t* graph, int src, int dest) ;
// 删除顶点
void nv_graph_remove_vertex(nv_graph_t* graph, int id) ;

// 删除边
void nv_graph_remove_edge(nv_graph_t* graph, int src, int dest) ;

// 遍历图
void nv_graph_traverse(nv_graph_t* graph) ;



int nv_graph_main() ;


    
#ifdef __cplusplus
}
#endif

#endif 