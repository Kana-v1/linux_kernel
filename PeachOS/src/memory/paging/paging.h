//
// Created by kana on 7/19/23.
//

#ifndef LINUX_KERNEL_PAGING_H
#define LINUX_KERNEL_PAGING_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// page table entry: https://wiki.osdev.org/images/6/60/Page_table_entry.png

#define PAGING_CACHE_DISABLED   0b00010000
#define PAGING_WRITE_THROUGH    0b00001000
#define PAGING_ACCESS_FROM_ALL  0b00000100  // all supervisor rings can access page
#define PAGING_IS_WRITEABLE     0b00000010
#define PAGING_IS_PRESENT       0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE  1024

#define PAGING_PAGE_SIZE    4096

struct Paging4GbChunk {
    uint32_t* directory_entry;
};

struct Paging4GbChunk* paging_new_4gb(uint8_t flags);

void paging_switch(uint32_t* directory);

void enable_paging(void);

uint32_t* paging_4gb_chunk_get_directory(struct Paging4GbChunk* chunk);

void paging_load_directory(uint32_t* directory);

bool paging_is_aligned(void* addr);

// val - physical address along with flags
int paging_set(uint32_t* directory, void* virtual_address, uint32_t val);

#endif //LINUX_KERNEL_PAGING_H
