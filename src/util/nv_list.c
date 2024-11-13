#include <stdio.h>
#include <stdlib.h>
#include "nv_list.h"

// 定义节点结构
typedef struct Node {
    int data;
    struct Node* prev;
    struct Node* next;
} Node;

// 创建新节点
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("内存分配失败\n");
        return NULL;
    }
    newNode->data = data;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// 插入节点到链表尾部
Node* insertEnd(Node* head, int data) {
    Node* newNode = createNode(data);
    if (newNode == NULL) return head;

    if (head == NULL) {
        head = newNode;
        return head;
    }

    Node* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = newNode;
    newNode->prev = temp;
    return head;
}

// 删除节点
Node* deleteNode(Node* head, int key) {
    if (head == NULL) return NULL;

    Node* temp = head;
    while (temp != NULL && temp->data != key) {
        temp = temp->next;
    }

    if (temp == NULL) return head;

    if (temp->prev != NULL) temp->prev->next = temp->next;
    if (temp->next != NULL) temp->next->prev = temp->prev;

    if (temp == head) head = temp->next;

    free(temp);
    return head;
}

// 打印链表
void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}
