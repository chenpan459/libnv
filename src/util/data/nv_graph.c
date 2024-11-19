#include "nv_graph.h"

/*
jaisawal
图(Graph)的数据结构

GitHub Copilot
要封装一个图（Graph）数据结构，可以定义一个图结构体，并提供基本的操作函数，如添加顶点、添加边、删除顶点、删除边和遍历图。以下是一个示例：

计划
定义图的顶点和边结构体。
定义图结构体。
实现初始化图、销毁图、添加顶点、添加边、删除顶点、删除边和遍历图的函数。
*/

// 初始化图
nv_graph_t* nv_graph_init() {
    nv_graph_t* graph = (nv_graph_t*)malloc(sizeof(nv_graph_t));
    graph->vertices = NULL;
    return graph;
}

// 销毁图
void nv_graph_destroy(nv_graph_t* graph) {
    nv_vertex_t* current_vertex = graph->vertices;
    while (current_vertex) {
        nv_edge_t* current_edge = current_vertex->edges;
        while (current_edge) {
            nv_edge_t* temp_edge = current_edge;
            current_edge = current_edge->next;
            free(temp_edge);
        }
        nv_vertex_t* temp_vertex = current_vertex;
        current_vertex = current_vertex->next;
        free(temp_vertex);
    }
    free(graph);
}

// 添加顶点
void nv_graph_add_vertex(nv_graph_t* graph, int id) {
    nv_vertex_t* new_vertex = (nv_vertex_t*)malloc(sizeof(nv_vertex_t));
    new_vertex->id = id;
    new_vertex->edges = NULL;
    new_vertex->next = graph->vertices;
    graph->vertices = new_vertex;
}

// 添加边
void nv_graph_add_edge(nv_graph_t* graph, int src, int dest) {
    nv_vertex_t* vertex = graph->vertices;
    while (vertex && vertex->id != src) {
        vertex = vertex->next;
    }
    if (vertex) {
        nv_edge_t* new_edge = (nv_edge_t*)malloc(sizeof(nv_edge_t));
        new_edge->dest = dest;
        new_edge->next = vertex->edges;
        vertex->edges = new_edge;
    }
}

// 删除顶点
void nv_graph_remove_vertex(nv_graph_t* graph, int id) {
    nv_vertex_t* current_vertex = graph->vertices;
    nv_vertex_t* prev_vertex = NULL;
    while (current_vertex && current_vertex->id != id) {
        prev_vertex = current_vertex;
        current_vertex = current_vertex->next;
    }
    if (current_vertex) {
        if (prev_vertex) {
            prev_vertex->next = current_vertex->next;
        } else {
            graph->vertices = current_vertex->next;
        }
        nv_edge_t* current_edge = current_vertex->edges;
        while (current_edge) {
            nv_edge_t* temp_edge = current_edge;
            current_edge = current_edge->next;
            free(temp_edge);
        }
        free(current_vertex);
    }
}

// 删除边
void nv_graph_remove_edge(nv_graph_t* graph, int src, int dest) {
    nv_vertex_t* vertex = graph->vertices;
    while (vertex && vertex->id != src) {
        vertex = vertex->next;
    }
    if (vertex) {
        nv_edge_t* current_edge = vertex->edges;
        nv_edge_t* prev_edge = NULL;
        while (current_edge && current_edge->dest != dest) {
            prev_edge = current_edge;
            current_edge = current_edge->next;
        }
        if (current_edge) {
            if (prev_edge) {
                prev_edge->next = current_edge->next;
            } else {
                vertex->edges = current_edge->next;
            }
            free(current_edge);
        }
    }
}

// 遍历图
void nv_graph_traverse(nv_graph_t* graph) {
    nv_vertex_t* vertex = graph->vertices;
    while (vertex) {
        printf("Vertex %d:\n", vertex->id);
        nv_edge_t* edge = vertex->edges;
        while (edge) {
            printf("  Edge to %d\n", edge->dest);
            edge = edge->next;
        }
        vertex = vertex->next;
    }
}

int nv_graph_main() {
    nv_graph_t* graph = nv_graph_init();

    nv_graph_add_vertex(graph, 1);
    nv_graph_add_vertex(graph, 2);
    nv_graph_add_vertex(graph, 3);

    nv_graph_add_edge(graph, 1, 2);
    nv_graph_add_edge(graph, 1, 3);
    nv_graph_add_edge(graph, 2, 3);

    nv_graph_traverse(graph);

    nv_graph_remove_edge(graph, 1, 2);
    nv_graph_remove_vertex(graph, 3);

    nv_graph_traverse(graph);

    nv_graph_destroy(graph);
    return 0;
}