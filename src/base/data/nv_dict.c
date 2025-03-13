#include "nv_dict.h"


// 哈希函数
// 定义一个静态函数hash，用于计算字符串的哈希值
static unsigned int hash(const char* key) {
    // 初始化哈希值为0
    unsigned int hash = 0;
    // 遍历字符串中的每一个字符，直到字符串结束符'\0'
    while (*key) {
        // 将当前哈希值左移5位，然后加上当前字符的ASCII值
        // 左移5位相当于乘以32，这样可以快速扩大哈希值的范围
        hash = (hash << 5) + *key++;
    }
    // 返回哈希值对TABLE_SIZE取模的结果
    // TABLE_SIZE通常是一个质数，用于减少哈希冲突
    return hash % TABLE_SIZE;
}

// 初始化字典
// 定义一个函数，用于初始化一个nv_dict_t类型的字典
nv_dict_t* nv_dict_init() {
    // 分配内存空间给nv_dict_t类型的指针dict，并确保分配成功
    nv_dict_t* dict = (nv_dict_t*)malloc(sizeof(nv_dict_t));
    // 分配内存空间给字典的buckets数组，大小为TABLE_SIZE个nv_dict_node_t指针
    dict->buckets = (nv_dict_node_t**)malloc(TABLE_SIZE * sizeof(nv_dict_node_t*));
    // 遍历buckets数组，将每个元素初始化为NULL，表示每个桶都是空的
    for (int i = 0; i < TABLE_SIZE; i++) {
        dict->buckets[i] = NULL;
    }
    // 返回初始化好的字典指针
    return dict;
}

// 销毁字典
// 定义一个函数，用于销毁一个nv_dict_t类型的字典
void nv_dict_destroy(nv_dict_t* dict) {
    // 遍历字典中的所有桶（假设桶的数量为TABLE_SIZE）
    for (int i = 0; i < TABLE_SIZE; i++) {
        // 获取当前桶的第一个节点
        nv_dict_node_t* node = dict->buckets[i];
        // 遍历当前桶中的所有节点
        while (node) {
            // 临时保存当前节点
            nv_dict_node_t* temp = node;
            // 移动到下一个节点
            node = node->next;
            // 释放当前节点的键值内存
            free(temp->key);
            // 释放当前节点本身的内存
            free(temp);
        }
    }
    // 释放存储桶的数组内存
    free(dict->buckets);
    // 释放字典结构本身的内存
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
// 函数声明：nv_dict_remove 用于从字典中移除指定的键值对
void nv_dict_remove(nv_dict_t* dict, const char* key) {
    // 计算键的哈希值，用于确定其在哈希表中的位置
    unsigned int index = hash(key);
    // 获取哈希表中对应位置的链表头节点
    nv_dict_node_t* node = dict->buckets[index];
    // 初始化前一个节点指针为NULL
    nv_dict_node_t* prev = NULL;
    // 遍历链表，查找与指定键相匹配的节点
    while (node) {
        // 如果当前节点的键与指定键相等
        if (strcmp(node->key, key) == 0) {
            // 如果前一个节点存在，则将前一个节点的next指针指向当前节点的下一个节点
            if (prev) {
                prev->next = node->next;
            } else {
                // 如果前一个节点不存在，说明当前节点是链表的头节点，直接更新头节点
                dict->buckets[index] = node->next;
            }
            // 释放当前节点的键和节点本身的内存
            free(node->key);
            free(node);
            // 退出函数
            return;
        }
        // 更新前一个节点为当前节点
        prev = node;
        // 移动到下一个节点
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

// 定义一个名为nv_dict_main的函数，用于演示字典的使用
int nv_dict_main() {
    // 初始化一个字典，返回字典的指针
    nv_dict_t* dict = nv_dict_init();

    // 向字典中插入键值对，键为"key1"，值为"value1"
    nv_dict_insert(dict, "key1", "value1");
    // 向字典中插入键值对，键为"key2"，值为"value2"
    nv_dict_insert(dict, "key2", "value2");

    // 查找键为"key1"的值，并将结果存储在value指针中
    char* value = (char*)nv_dict_lookup(dict, "key1");
    // 如果找到了值，打印"Found: "和找到的值
    if (value) {
        printf("Found: %s\n", value);
    } else {
        // 如果没有找到值，打印"Not found"
        printf("Not found\n");
    }

    // 从字典中移除键为"key1"的键值对
    nv_dict_remove(dict, "key1");
    // 再次查找键为"key1"的值
    value = (char*)nv_dict_lookup(dict, "key1");
    // 如果找到了值，打印"Found: "和找到的值
    if (value) {
        printf("Found: %s\n", value);
    } else {
        // 如果没有找到值，打印"Not found"
        printf("Not found\n");
    }

    // 销毁字典，释放相关资源
    nv_dict_destroy(dict);
    // 返回0，表示程序正常结束
    return 0;
}