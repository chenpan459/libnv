#include "nv_rb_tree.h"

// 创建新节点
// 定义一个函数，用于创建一个新的红黑树节点
nv_rb_Node* nv_rb_newNode(int data) {
    // 使用malloc函数为新的节点分配内存，大小为nv_rb_Node结构体的大小
    nv_rb_Node* node = (nv_rb_Node*)malloc(sizeof(nv_rb_Node));
    // 将传入的数据赋值给节点的data字段
    node->data = data;
    // 将节点的颜色设置为红色，RED通常定义为1或某个宏
    node->color = RED;
    // 初始化节点的左子节点、右子节点和父节点为NULL，表示这些指针当前没有指向任何节点
    node->left = node->right = node->parent = NULL;
    // 返回创建的节点指针
    return node;
}

// 左旋
// 定义一个函数，用于对红黑树进行左旋操作
// 参数root是红黑树的根节点，参数pt是需要进行左旋操作的节点
nv_rb_Node* nv_rb_leftRotate(nv_rb_Node* root, nv_rb_Node* pt) {
    // 获取pt节点的右子节点
    nv_rb_Node* pt_right = pt->right;
    // 将pt的右子节点设置为pt_right的左子节点
    pt->right = pt_right->left;

    // 如果pt_right的左子节点不为空，则将其父节点设置为pt
    if (pt->right != NULL)
        pt->right->parent = pt;

    // 将pt_right的父节点设置为pt的父节点
    pt_right->parent = pt->parent;

    // 如果pt的父节点为空，说明pt是根节点，此时将根节点设置为pt_right
    if (pt->parent == NULL)
        root = pt_right;
    // 如果pt是其父节点的左子节点，则将pt的父节点的左子节点设置为pt_right
    else if (pt == pt->parent->left)
        pt->parent->left = pt_right;
    // 否则，将pt的父节点的右子节点设置为pt_right
    else
        pt->parent->right = pt_right;

    // 将pt_right的左子节点设置为pt
    pt_right->left = pt;
    // 将pt的父节点设置为pt_right
    pt->parent = pt_right;
    // 返回新的根节点
    return root;
}

// 右旋
// 定义一个函数，用于对红黑树进行右旋操作
// 参数root是红黑树的根节点，pt是需要进行右旋的节点
nv_rb_Node* nv_rb_rightRotate(nv_rb_Node* root, nv_rb_Node* pt) {
    // 获取pt节点的左子节点
    nv_rb_Node* pt_left = pt->left;
    // 将pt的左子节点设置为pt_left的右子节点
    pt->left = pt_left->right;

    // 如果pt_left的右子节点不为空，将其父节点设置为pt
    if (pt->left != NULL)
        pt->left->parent = pt;

    // 将pt_left的父节点设置为pt的父节点
    pt_left->parent = pt->parent;

    // 如果pt的父节点为空，说明pt是根节点，将根节点设置为pt_left
    if (pt->parent == NULL)
        root = pt_left;
    // 如果pt是其父节点的左子节点，将pt的父节点的左子节点设置为pt_left
    else if (pt == pt->parent->left)
        pt->parent->left = pt_left;
    // 否则，将pt的父节点的右子节点设置为pt_left
    else
        pt->parent->right = pt_left;

    // 将pt_left的右子节点设置为pt
    pt_left->right = pt;
    // 将pt的父节点设置为pt_left
    pt->parent = pt_left;
    // 返回新的根节点
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

