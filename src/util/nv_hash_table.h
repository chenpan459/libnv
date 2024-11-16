#ifndef _NV_HASH_TABLE_H_INCLUDED_
#define _NV_HASH_TABLE_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100  // 哈希表大小



// 哈希表节点结构
typedef struct HashNode {
    char* key;
    char* value;
    struct HashNode* next;  // 解决冲突的链表
} HashNode;

// 哈希表结构
typedef struct HashTable {
    HashNode** nodes;
    int size;
} HashTable;


HashTable* nv_hash_table_create(int size);
void nv_hash_table_insert(HashTable* table, const char* key, const char* value);

char* nv_hash_table_find(HashTable* table, const char* key);
void nv_hash_table_delete(HashTable* table, const char* key);
void nv_hash_table_destroy(HashTable* table);






#endif 