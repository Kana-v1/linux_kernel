//
// Created by kana on 7/11/23.
//

#ifndef LINUX_KERNEL_KERNEL_H
#define LINUX_KERNEL_KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include "io/io.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "string/string.h"
#include "disk/streamer.h"
#include "fs/file.h"

#define VGA_WIDTH       80
#define VGA_HEIGHT      20

#define PEACHOS_MAX_PATH    100

void kernel_main(void);

void print(const char* str);

#define ERROR(value)        (void*)(value)
#define ERROR_I(value)      (int)(value)
#define IS_ERROR(value)     ((int)value) < 0

#endif //LINUX_KERNEL_KERNEL_H
