//
// Created by kana on 7/17/23.
//

#ifndef LINUX_KERNEL_HEAP_H
#define LINUX_KERNEL_HEAP_H

#include "../../config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00

#define HEAP_BLOCK_HAS_NEXT     0b100000000
#define HEAP_BLOCK_IS_FIRST      0b010000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct HeapTable {
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};

struct Heap {
    struct HeapTable* table;
    void* startAddr;
};

int heap_create(struct Heap* heap, void* start, void* end, struct HeapTable* table);

void* heap_malloc(struct Heap* heap, size_t size);

void heap_free(struct Heap* heap, void* start);

#endif //LINUX_KERNEL_HEAP_H
