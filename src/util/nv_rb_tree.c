


#include <stdio.h>
#include <stdlib.h>

// 定义颜色
#define RED   0
#define BLACK 1

// 定义节点结构
typedef struct Node {
    int data;
    int color;
    struct Node *left, *right, *parent;
} Node;

// 创建新节点
Node* newNode(int data) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = data;
    node->color = RED;
    node->left = node->right = node->parent = NULL;
    return node;
}

// 左旋
Node* leftRotate(Node* root, Node* pt) {
    Node* pt_right = pt->right;
    pt->right = pt_right->left;

    if (pt->right != NULL)
        pt->right->parent = pt;

    pt_right->parent = pt->parent;

    if (pt->parent == NULL)
        root = pt_right;
    else if (pt == pt->parent->left)
        pt->parent->left = pt_right;
    else
        pt->parent->right = pt_right;

    pt_right->left = pt;
    pt->parent = pt_right;
    return root;
}

// 右旋
Node* rightRotate(Node* root, Node* pt) {
    Node* pt_left = pt->left;
    pt->left = pt_left->right;

    if (pt->left != NULL)
        pt->left->parent = pt;

    pt_left->parent = pt->parent;

    if (pt->parent == NULL)
        root = pt_left;
    else if (pt == pt->parent->left)
        pt->parent->left = pt_left;
    else
        pt->parent->right = pt_left;

    pt_left->right = pt;
    pt->parent = pt_left;
    return root;
}

// 插入修复
Node* insertFix(Node* root, Node* pt) {
    Node* parent_pt = NULL;
    Node* grand_parent_pt = NULL;

    while ((pt != root) && (pt->color != BLACK) &&
           (pt->parent->color == RED)) {

        parent_pt = pt->parent;
        grand_parent_pt = pt->parent->parent;

        //  Case : A
        if (parent_pt == grand_parent_pt->left) {
            Node* uncle_pt = grand_parent_pt->right;

            // Case : 1
            if (uncle_pt != NULL && uncle_pt->color == RED) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            }

            // Case : 2
            else {
                if (pt == parent_pt->right) {
                    root = leftRotate(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }
                root = rightRotate(root, grand_parent_pt);
                int t = parent_pt->color;
                parent_pt->color = grand_parent_pt->color;
                grand_parent_pt->color = t;
                pt = parent_pt;
            }
        }

        // Case : B
        else {
            Node* uncle_pt = grand_parent_pt->left;

            // Case : 1
            if ((uncle_pt != NULL) && (uncle_pt->color == RED)) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            }

            // Case : 2
            else {
                if (pt == parent_pt->left) {
                    root = rightRotate(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }
                root = leftRotate(root, grand_parent_pt);
                int t = parent_pt->color;
                parent_pt->color = grand_parent_pt->color;
                grand_parent_pt->color = t;
                pt = parent_pt;
            }
        }
    }

    root->color = BLACK;
    return root;
}

// BST插入
Node* insertBST(Node* root, Node* pt) {
    if (root == NULL)
        return pt;

    if (pt->data < root->data) {
        root->left = insertBST(root->left, pt);
        root->left->parent = root;
    } else if (pt->data > root->data) {
        root->right = insertBST(root->right, pt);
        root->right->parent = root;
    }

    return root;
}
// 插入节点
Node* insert(Node* root, int data) {
    Node* pt = newNode(data);

    // Perform normal BST insertion
    root = insertBST(root, pt);

    // Fix the tree
    root = insertFix(root, pt);
    return root;
}

// 查找节点
Node* search(Node* root, int data) {
    if (root == NULL || root->data == data)
        return root;

    if (root->data < data)
        return search(root->right, data);

    return search(root->left, data);
}

// 中序遍历
void inorder(Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d  ", root->data);
        inorder(root->right);
    }
}



