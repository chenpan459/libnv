
#include "nv_set.h"



// 初始化集合
// 定义一个函数nv_set_init，用于初始化一个nv_set_t类型的集合
nv_set_t* nv_set_init() {
    // 使用malloc函数分配内存，大小为nv_set_t结构体的大小，并将返回的指针转换为nv_set_t*类型
    nv_set_t* set = (nv_set_t*)malloc(sizeof(nv_set_t));
    // 将集合的头指针初始化为NULL，表示集合为空
    set->head = NULL;
    // 将集合的大小初始化为0，表示集合中没有元素
    set->size = 0;
    // 返回初始化后的集合指针
    return set;
}

// 销毁集合
// 定义一个函数，用于销毁一个nv_set_t类型的集合
void nv_set_destroy(nv_set_t* set) {
    // 初始化一个指针current，指向集合的头部节点
    nv_set_node_t* current = set->head;
    // 使用while循环遍历集合中的所有节点
    while (current) {
        nv_set_node_t* temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    free(set);
}

// 检查元素是否存在
bool nv_set_contains(nv_set_t* set, const char* data) {
    nv_set_node_t* current = set->head;
    while (current) {
        if (strcmp(current->data, data) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// 添加元素
// 函数声明：向非重复集合中添加元素
// 参数：
//   set: 指向非重复集合的指针
//   data: 要添加的字符串数据
void nv_set_add(nv_set_t* set, const char* data) {
    // 检查集合中是否已经包含该元素
    if (nv_set_contains(set, data)) {
        return;  // 元素已存在
    }
    nv_set_node_t* new_node = (nv_set_node_t*)malloc(sizeof(nv_set_node_t));
    new_node->data = strdup(data);
    new_node->next = set->head;
    set->head = new_node;
    set->size++;
}

// 删除元素
// 函数声明：nv_set_remove
// 功能：从给定的集合中移除指定的数据
// 参数：
//   set - 指向集合的指针
//   data - 指向要移除数据的指针
void nv_set_remove(nv_set_t* set, const char* data) {
    // 初始化当前节点指针为集合的头节点
    nv_set_node_t* current = set->head;
    // 初始化前一个节点指针为NULL
    nv_set_node_t* prev = NULL;
    // 遍历集合中的节点，直到找到匹配的数据或遍历完整个集合
    while (current) {
        // 比较当前节点的数据与要移除的数据
        if (strcmp(current->data, data) == 0) {
            // 如果找到匹配的数据，执行以下操作：
            if (prev) {
                // 如果当前节点不是头节点，将前一个节点的next指针指向当前节点的下一个节点
                prev->next = current->next;
            } else {
                // 如果当前节点是头节点，将集合的头指针指向当前节点的下一个节点
                set->head = current->next;
            }
            // 释放当前节点数据的内存
            free(current->data);
            // 释放当前节点的内存
            free(current);
            // 集合大小减1
            set->size--;
            // 退出函数
            return;
        }
        // 更新前一个节点指针为当前节点
        prev = current;
        // 更新当前节点指针为下一个节点
        current = current->next;
    }
}

// 获取集合大小
int nv_set_size(nv_set_t* set) {
    return set->size;
}

int nv_set_main() {
    nv_set_t* set = nv_set_init();

    nv_set_add(set, "element1");
    nv_set_add(set, "element2");
    nv_set_add(set, "element3");

    printf("Set contains 'element2': %s\n", nv_set_contains(set, "element2") ? "true" : "false");
    printf("Set size: %d\n", nv_set_size(set));

    nv_set_remove(set, "element2");

    printf("Set contains 'element2': %s\n", nv_set_contains(set, "element2") ? "true" : "false");
    printf("Set size: %d\n", nv_set_size(set));

    nv_set_destroy(set);
    return 0;
}