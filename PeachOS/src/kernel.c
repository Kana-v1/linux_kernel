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

void panic(const char* msg) {
	print("\n");
	print(msg);
	while(1){}
}

struct Tss tss;
struct Gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct GdtStructured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
		{.base = 0x00, .limit = 0x00, .type = 0x00},					// NULL segment
		{.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A},				// kernel code segment
		{.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92},				// kernel data segment
		{.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8},				// user code segment
		{.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2},				// user data segment
		{.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9},	// tss segment
};


void kernel_main(void) {
    terminal_initialize();
    print("Hello world!");

	memset(gdt_real, 0x00, sizeof(gdt_real));
	gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);

	// load the gdt
	gdt_load(gdt_real, sizeof(gdt_real));

    // initialize the heap
    kheap_init();

    // initialize file system
    fs_init();

    // search and initialize the disks
    disk_search_and_init();

    // initialize the interrupt descriptor table
    idt_init();

	// set up the tss
	memset(&tss, 0x00, sizeof(tss));
	tss.esp0 = 0x600000;
	tss.ss0 = KERNEL_DATA_SELECTOR;

	tss_load(0x28); // 0x28 - the offset in the gdt_real


    // set up paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    enable_paging();

    // enable system interrupts
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        struct FileStat s;
		fstat(fd, &s);
		fclose(fd);
    }

    while (1) {}
}