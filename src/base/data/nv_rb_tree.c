#include "nv_rb_tree.h"

// 创建新节点
nv_rb_Node* nv_rb_newNode(int data) {
    nv_rb_Node* node = (nv_rb_Node*)malloc(sizeof(nv_rb_Node));
    node->data = data;
    node->color = RED;
    node->left = node->right = node->parent = NULL;
    return node;
}

// 左旋
nv_rb_Node* nv_rb_leftRotate(nv_rb_Node* root, nv_rb_Node* pt) {
    nv_rb_Node* pt_right = pt->right;
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
nv_rb_Node* nv_rb_rightRotate(nv_rb_Node* root, nv_rb_Node* pt) {
    nv_rb_Node* pt_left = pt->left;
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
nv_rb_Node* nv_rb_insertFix(nv_rb_Node* root, nv_rb_Node* pt) {
    nv_rb_Node* parent_pt = NULL;
    nv_rb_Node* grand_parent_pt = NULL;

    while ((pt != root) && (pt->color != BLACK) &&
           (pt->parent->color == RED)) {

        parent_pt = pt->parent;
        grand_parent_pt = pt->parent->parent;

        //  Case : A
        if (parent_pt == grand_parent_pt->left) {
            nv_rb_Node* uncle_pt = grand_parent_pt->right;

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
                    root = nv_rb_leftRotate(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }
                root = nv_rb_rightRotate(root, grand_parent_pt);
                int t = parent_pt->color;
                parent_pt->color = grand_parent_pt->color;
                grand_parent_pt->color = t;
                pt = parent_pt;
            }
        }

        // Case : B
        else {
            nv_rb_Node* uncle_pt = grand_parent_pt->left;

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
                    root = nv_rb_rightRotate(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }
                root = nv_rb_leftRotate(root, grand_parent_pt);
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
nv_rb_Node* nv_rb_insertBST(nv_rb_Node* root, nv_rb_Node* pt) {
    if (root == NULL)
        return pt;

    if (pt->data < root->data) {
        root->left = nv_rb_insertBST(root->left, pt);
        root->left->parent = root;
    } else if (pt->data > root->data) {
        root->right = nv_rb_insertBST(root->right, pt);
        root->right->parent = root;
    }

    return root;
}
// 插入节点
nv_rb_Node* nv_rb_insert(nv_rb_Node* root, int data) {
    nv_rb_Node* pt = nv_rb_newNode(data);

    // Perform normal BST insertion
    root = nv_rb_insertBST(root, pt);

    // Fix the tree
    root = nv_rb_insertFix(root, pt);
    return root;
}

// 查找节点
nv_rb_Node* nv_rb_search(nv_rb_Node* root, int data) {
    if (root == NULL || root->data == data)
        return root;

    if (root->data < data)
        return nv_rb_search(root->right, data);

    return nv_rb_search(root->left, data);
}

// 中序遍历
void nv_rb_inorder(nv_rb_Node* root) {
    if (root != NULL) {
        nv_rb_inorder(root->left);
        printf("%d  ", root->data);
        nv_rb_inorder(root->right);
    }
}




int nv_rb_main(){

    nv_rb_Node* root = NULL;
    root = nv_rb_insert(root, 7);
    root = nv_rb_insert(root, 6);
    root = nv_rb_insert(root, 5);
    root = nv_rb_insert(root, 4);
    root = nv_rb_insert(root, 3);
    root = nv_rb_insert(root, 2);
    root = nv_rb_insert(root, 1);

    printf("中序遍历结果: ");
    nv_rb_inorder(root);

    int num = 4;
    nv_rb_Node* res = nv_rb_search(root, num);
    if (res != NULL)
        printf("\n元素 %d 在树中\n", num);
    else
        printf("\n元素 %d 不在树中\n", num);

    return 0;
    
}

