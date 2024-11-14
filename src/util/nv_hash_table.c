
#include "nv_hash_table.h"

// 哈希函数
unsigned int hash(const char* key, int size) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % size;
}

// 创建哈希表
HashTable* nv_hash_table_create(int size) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = size;
    table->nodes = (HashNode**)calloc(size, sizeof(HashNode*));
    return table;
}

// 插入键值对
void nv_hash_table_insert(HashTable* table, const char* key, const char* value) {
    unsigned int index = hash(key, table->size);
    HashNode* list = table->nodes[index];

    // 检查键是否已存在
    while (list != NULL) {
        if (strcmp(list->key, key) == 0) {
            // 更新值
            free(list->value);
            list->value = strdup(value);
            return;
        }
        list = list->next;
    }

    // 创建新节点
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->key = strdup(key);
    newNode->value = strdup(value);
    newNode->next = table->nodes[index];
    table->nodes[index] = newNode;
}

// 查找键值
char* nv_hash_table_find(HashTable* table, const char* key) {
    unsigned int index = hash(key, table->size);
    HashNode* list = table->nodes[index];

    while (list != NULL) {
        if (strcmp(list->key, key) == 0) {
            return list->value;
        }
        list = list->next;
    }

    return NULL;  // 未找到
}

// 删除键值对
void nv_hash_table_delete(HashTable* table, const char* key) {
    unsigned int index = hash(key, table->size);
    HashNode* list = table->nodes[index];
    HashNode* prev = NULL;

    while (list != NULL) {
        if (strcmp(list->key, key) == 0) {
            if (prev == NULL) {
                // 删除头节点
                table->nodes[index] = list->next;
            } else {
                prev->next = list->next;
            }
            free(list->key);
            free(list->value);
            free(list);
            return;
        }
        prev = list;
        list = list->next;
    }
}

// 销毁哈希表
void nv_hash_table_destroy(HashTable* table) {
    for (int i = 0; i < table->size; ++i) {
        HashNode* list = table->nodes[i];
        while (list != NULL) {
            HashNode* temp = list;
            list = list->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(table->nodes);
    free(table);
}



int nv_hash_table_main()
{

    
    HashTable* table = nv_hash_table_create(HASH_SIZE);

    nv_hash_table_insert(table, "key1", "value1");
    nv_hash_table_insert(table, "key2", "value2");
    nv_hash_table_insert(table, "key3", "value3");

    printf("key1: %s\n", nv_hash_table_find(table, "key1"));
    printf("key2: %s\n", nv_hash_table_find(table, "key2"));
    printf("key3: %s\n", nv_hash_table_find(table, "key3"));

    nv_hash_table_delete(table, "key2");

    printf("key2: %s\n", nv_hash_table_find(table, "key2"));

    nv_hash_table_destroy(table);
    return 0;
}