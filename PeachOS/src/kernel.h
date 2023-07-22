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

#define VGA_WIDTH       80
#define VGA_HEIGHT      20

#define PEACHOS_MAX_PATH    100

void kernel_main(void);

void print(const char* str);

#endif //LINUX_KERNEL_KERNEL_H
