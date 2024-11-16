#include <stdio.h>
#include <stdlib.h>
#include "nv_list.h"


// 创建新节点
nv_list_Node* nv_list_createNode(int data) {
    nv_list_Node* newNode = (nv_list_Node*)malloc(sizeof(nv_list_Node));
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
nv_list_Node* nv_list_insertEnd(nv_list_Node* head, int data) {
    nv_list_Node* newNode = nv_list_createNode(data);
    if (newNode == NULL) return head;

    if (head == NULL) {
        head = newNode;
        return head;
    }

    nv_list_Node* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = newNode;
    newNode->prev = temp;
    return head;
}

// 删除节点
nv_list_Node* nv_list_deleteNode(nv_list_Node* head, int key) {
    if (head == NULL) return NULL;

    nv_list_Node* temp = head;
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
void nv_list_printList(nv_list_Node* head) {
    nv_list_Node* temp = head;
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}
