//
// Created by kana on 7/19/23.
//

#include "paging.h"
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
        directory[i] = (uint32_t)page_table_entry | flags | PAGING_IS_WRITEABLE;
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
