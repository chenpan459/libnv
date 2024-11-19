#include "nv_dict.h"


// 哈希函数
static unsigned int hash(const char* key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % TABLE_SIZE;
}

// 初始化字典
nv_dict_t* nv_dict_init() {
    nv_dict_t* dict = (nv_dict_t*)malloc(sizeof(nv_dict_t));
    dict->buckets = (nv_dict_node_t**)malloc(TABLE_SIZE * sizeof(nv_dict_node_t*));
    for (int i = 0; i < TABLE_SIZE; i++) {
        dict->buckets[i] = NULL;
    }
    return dict;
}

// 销毁字典
void nv_dict_destroy(nv_dict_t* dict) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        nv_dict_node_t* node = dict->buckets[i];
        while (node) {
            nv_dict_node_t* temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(dict->buckets);
    free(dict);
}

// 插入键值对
void nv_dict_insert(nv_dict_t* dict, const char* key, void* value) {
    unsigned int index = hash(key);
    nv_dict_node_t* new_node = (nv_dict_node_t*)malloc(sizeof(nv_dict_node_t));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = dict->buckets[index];
    dict->buckets[index] = new_node;
}

// 查找值
void* nv_dict_lookup(nv_dict_t* dict, const char* key) {
    unsigned int index = hash(key);
    nv_dict_node_t* node = dict->buckets[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

// 删除键值对
void nv_dict_remove(nv_dict_t* dict, const char* key) {
    unsigned int index = hash(key);
    nv_dict_node_t* node = dict->buckets[index];
    nv_dict_node_t* prev = NULL;
    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                dict->buckets[index] = node->next;
            }
            free(node->key);
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

// 检查字典是否为空
bool nv_dict_is_empty(nv_dict_t* dict) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (dict->buckets[i] != NULL) {
            return false;
        }
    }
    return true;
}

int nv_dict_main() {
    nv_dict_t* dict = nv_dict_init();

    nv_dict_insert(dict, "key1", "value1");
    nv_dict_insert(dict, "key2", "value2");

    char* value = (char*)nv_dict_lookup(dict, "key1");
    if (value) {
        printf("Found: %s\n", value);
    } else {
        printf("Not found\n");
    }

    nv_dict_remove(dict, "key1");
    value = (char*)nv_dict_lookup(dict, "key1");
    if (value) {
        printf("Found: %s\n", value);
    } else {
        printf("Not found\n");
    }

    nv_dict_destroy(dict);
    return 0;
}