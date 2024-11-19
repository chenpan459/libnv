#include "nv_heap.h"



/*要封装一个堆（Heap）数据结构，可以定义一个堆结构体，并提供基本的操作函数，如插入元素、删除元素、获取堆顶元素和检查堆是否为空。以下是一个示例：

计划
定义堆结构体。
实现初始化堆、销毁堆、插入元素、删除堆顶元素、获取堆顶元素和检查堆是否为空的函数。
实现堆的上滤和下滤操作。
*/
// 初始化堆
nv_heap_t* nv_heap_init() {
    nv_heap_t* heap = (nv_heap_t*)malloc(sizeof(nv_heap_t));
    heap->data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    heap->size = 0;
    heap->capacity = INITIAL_CAPACITY;
    return heap;
}

// 销毁堆
void nv_heap_destroy(nv_heap_t* heap) {
    free(heap->data);
    free(heap);
}

// 检查堆是否为空
bool nv_heap_is_empty(nv_heap_t* heap) {
    return heap->size == 0;
}

// 获取堆顶元素
int nv_heap_peek(nv_heap_t* heap) {
    if (nv_heap_is_empty(heap)) {
        fprintf(stderr, "Heap is empty\n");
        exit(EXIT_FAILURE);
    }
    return heap->data[0];
}

// 上滤操作
void nv_heap_sift_up(nv_heap_t* heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->data[index] <= heap->data[parent]) {
            break;
        }
        int temp = heap->data[index];
        heap->data[index] = heap->data[parent];
        heap->data[parent] = temp;
        index = parent;
    }
}

// 下滤操作
void nv_heap_sift_down(nv_heap_t* heap, int index) {
    while (2 * index + 1 < heap->size) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = left;
        if (right < heap->size && heap->data[right] > heap->data[left]) {
            largest = right;
        }
        if (heap->data[index] >= heap->data[largest]) {
            break;
        }
        int temp = heap->data[index];
        heap->data[index] = heap->data[largest];
        heap->data[largest] = temp;
        index = largest;
    }
}

// 插入元素
void nv_heap_insert(nv_heap_t* heap, int value) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->data = (int*)realloc(heap->data, heap->capacity * sizeof(int));
    }
    heap->data[heap->size] = value;
    nv_heap_sift_up(heap, heap->size);
    heap->size++;
}

// 删除堆顶元素
int nv_heap_remove(nv_heap_t* heap) {
    if (nv_heap_is_empty(heap)) {
        fprintf(stderr, "Heap is empty\n");
        exit(EXIT_FAILURE);
    }
    int root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    nv_heap_sift_down(heap, 0);
    return root;
}

int nv_heap_main() {
    nv_heap_t* heap = nv_heap_init();

    nv_heap_insert(heap, 10);
    nv_heap_insert(heap, 20);
    nv_heap_insert(heap, 5);
    nv_heap_insert(heap, 30);

    printf("Heap top: %d\n", nv_heap_peek(heap));

    while (!nv_heap_is_empty(heap)) {
        printf("Removed: %d\n", nv_heap_remove(heap));
    }

    nv_heap_destroy(heap);
    return 0;
}