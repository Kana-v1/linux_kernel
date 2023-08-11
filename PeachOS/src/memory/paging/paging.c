//
// Created by kana on 7/19/23.
//

#include "paging.h"
#include "../../status.h"
#include "../../memory/heap/kheap.h"


static uint32_t* current_directory = 0;

struct Paging4GbChunk* paging_new_4gb(uint8_t flags) {
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

    int offset = 0;

    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++) {
        uint32_t* page_table_entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++) {
            page_table_entry[j] = (offset + j * PAGING_PAGE_SIZE) | flags;
        }

        offset += PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        directory[i] = (uint32_t) page_table_entry | flags | PAGING_IS_WRITEABLE;
    }

    struct Paging4GbChunk* chunk_4gb = kzalloc(sizeof(struct Paging4GbChunk));
    chunk_4gb->directory_entry = directory;

    return chunk_4gb;
}

void paging_switch(uint32_t* directory) {
    paging_load_directory(directory);
    current_directory = directory;
}

uint32_t* paging_4gb_chunk_get_directory(struct Paging4GbChunk* chunk) {
    return chunk->directory_entry;
}

bool paging_is_aligned(void* addr) {
    return ((uint32_t) addr % PAGING_PAGE_SIZE) == 0;
}

int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out) {
    if (!paging_is_aligned(virtual_address)) {
        return EINVARG;
    }

    *directory_index_out = ((uint32_t) virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t) virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) /
                        PAGING_PAGE_SIZE);

    return PEACHOS_ALL_OK;
}

int paging_set(uint32_t *directory, void* virtual_address, uint32_t val) {
    if (!paging_is_aligned(virtual_address)) {
        return EINVARG;
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    int res = paging_get_indexes(virtual_address, &directory_index, &table_index);
    if (res < PEACHOS_ALL_OK) {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000); // extract only the address without flags

    table[table_index] = val;

    return PEACHOS_ALL_OK;
}

void paging_free_4gb(struct Paging4GbChunk* chunk) {
	for (int i = 0; i < 1024; i++) {
		uint32_t entry = chunk->directory_entry[i];
		uint32_t* table = (uint32_t*)(entry & 0xFFFF000); // last 3 bits are flags
		kfree(table);
	}

	kfree(chunk->directory_entry);
	kfree(chunk);
}
