//
// Created by kana on 7/17/23.
//

#include "kheap.h"
#include "heap.h"
#include "../../config.h"
#include "../../kernel.h"
#include "../memory.h"


struct Heap kernel_heap;
struct HeapTable kernel_heap_table;

void kheap_init(void) {
    int total_table_entries = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE; // 25600 bytes
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) (PEACHOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*) (PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*) (PEACHOS_HEAP_ADDRESS), end, &kernel_heap_table);

    if (res < 0) {
        print("\nfailed to create heap\n");
    }
}

void* kmalloc(size_t size) {
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size) {
    void* ptr = kmalloc(size);
    if (!ptr) {
        return 0;
    }

    memset(ptr, 0x00, size);

    return ptr;
}

void kfree(void* ptr) {
    heap_free(&kernel_heap, ptr);
}