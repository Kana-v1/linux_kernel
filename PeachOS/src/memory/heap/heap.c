//
// Created by kana on 7/17/23.
//

#include "heap.h"
#include "../../kernel.h"
#include <stdbool.h>

static int heap_validate_table(void* start, void* end, struct HeapTable* table) {
    int res = PEACHOS_ALL_OK;

    size_t table_size = (size_t)(end - start);
    size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE;
    if (table->total != total_blocks) {
        res = EINVARG;
    }

    return res;
}

static bool heap_validate_alignment(void* ptr) {
    return ((unsigned int) ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct Heap* heap, void* start, void* end, struct HeapTable* table) {
    int res = PEACHOS_ALL_OK;

    if (!heap_validate_alignment(start) || !heap_validate_alignment(end)) {
        res = EINVARG;
        return res;
    }

    memset(heap, 0, sizeof(struct Heap));
    heap->startAddr = start;
    heap->table = table;

    res = heap_validate_table(start, end, table);
    if (res < PEACHOS_ALL_OK) {
        return res;
    }

    size_t heap_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, heap_size);

    return res;
}

static uint32_t heap_align_value_to_upper(uint32_t value) {
    if (value % PEACHOS_HEAP_BLOCK_SIZE == 0) {
        return value;
    }

    value -= value % PEACHOS_HEAP_BLOCK_SIZE;
    value += PEACHOS_HEAP_BLOCK_SIZE;

    return value;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry) {
    return entry & 0b1111; // return last 4 bits
}

int heap_get_start_block(struct Heap* heap, uint32_t total_blocks) {
    struct HeapTable* table = heap->table;
    int bc = 0;    // current block
    int bs = -1;    // start block

    for (size_t i = 0; i < table->total; i++) {
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            bc = 0;
            bs = -1;
            continue;
        }

        // if this is the first block
        if (bs == -1) {
            bs = i;
        }

        if (++bc == total_blocks) {
            break;
        }
    }

    if (bs == -1) {
        return ENOMEM;
    }

    return bs;
}

void* heap_block_to_address(struct Heap* heap, uint32_t block) {
    return heap->startAddr + block * PEACHOS_HEAP_BLOCK_SIZE;
}

void heap_mark_blocks_taken(struct Heap* heap, uint32_t start_block, uint32_t total_blocks) {
    uint32_t end_block = start_block + total_blocks - 1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

    if (total_blocks > 1) {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block; i <= end_block; i++) {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block - 1) {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

void* heap_malloc_blocks(struct Heap* heap, uint32_t total_blocks) {
    void* address = 0;

    int start_block = heap_get_start_block(heap, total_blocks);

    if (start_block < 0) {
        return address;
    }

    address = heap_block_to_address(heap, start_block);

    // mark the block as taken
    heap_mark_blocks_taken(heap, start_block, total_blocks);

    return address;
}

void heap_mark_blocks_free(struct Heap* heap, int starting_block) {
    struct HeapTable* table = heap->table;
    for (int i = starting_block; i < (int) table->total; i++) {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break;
        }
    }
}

int heap_address_to_block(struct Heap* heap, void* addr) {
    return ((int) (addr - heap->startAddr)) / PEACHOS_HEAP_BLOCK_SIZE;
}

void* heap_malloc(struct Heap* heap, size_t size) {
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;

    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct Heap* heap, void* start) {
    heap_mark_blocks_free(heap, heap_address_to_block(heap, start));
}