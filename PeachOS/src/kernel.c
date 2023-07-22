//
// Created by kana on 7/11/23.
//
#include "kernel.h"

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char colour) {
    return (colour << 8) | c;
}

void terminal_put_char(int x, int y, char c, char colour) {
    video_mem[(y * VGA_WIDTH + x)] = terminal_make_char(c, colour);
}

void terminal_write_char(char c, char colour) {
    if (c == '\n') {
        terminal_row++;
        terminal_col = 0;
        return;
    }

    terminal_put_char(terminal_col++, terminal_row, c, colour);
    if (terminal_row >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
}

void terminal_initialize(void) {
    video_mem = (uint16_t * )(0xB8000);
    terminal_row = 0;
    terminal_col = 0;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            terminal_put_char(x, y, ' ', 0);
        }
    }
}

void print(const char* str) {
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        terminal_write_char(str[i], 15);
    }
}

static struct Paging4GbChunk* kernel_chunk = 0;

void kernel_main(void) {
    terminal_initialize();
    print("Hello world!\nSome test");

    // initialize the heap
    kheap_init();

    // search and initialize the disks
    disk_search_and_init();

    // initialize the interrupt descriptor table
    idt_init();

    // setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    uint32_t* kernel_directory = paging_4gb_chunk_get_directory(kernel_chunk);
    paging_switch(kernel_directory);

    enable_paging();

    // enable system interrupts
    enable_interrupts();

    struct PathRoot* root_path = pathparser_parse("0:/bin/shell.exe", NULL);
    if (root_path) {}
}