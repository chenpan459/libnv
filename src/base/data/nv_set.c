
#include "nv_set.h"



// 初始化集合
nv_set_t* nv_set_init() {
    nv_set_t* set = (nv_set_t*)malloc(sizeof(nv_set_t));
    set->head = NULL;
    set->size = 0;
    return set;
}

// 销毁集合
void nv_set_destroy(nv_set_t* set) {
    nv_set_node_t* current = set->head;
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
void nv_set_add(nv_set_t* set, const char* data) {
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
void nv_set_remove(nv_set_t* set, const char* data) {
    nv_set_node_t* current = set->head;
    nv_set_node_t* prev = NULL;
    while (current) {
        if (strcmp(current->data, data) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                set->head = current->next;
            }
            free(current->data);
            free(current);
            set->size--;
            return;
        }
        prev = current;
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