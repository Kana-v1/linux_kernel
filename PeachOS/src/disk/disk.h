//
// Created by kana on 7/21/23.
//

#ifndef LINUX_KERNEL_DISK_H
#define LINUX_KERNEL_DISK_H

#include "../status.h"
#include "../io/io.h"
#include "../memory/memory.h"
#include "../config.h"

typedef unsigned int PEACHOS_DISK_TYPE;

// a real physical hard disk
#define PEACH_OS_DISK_TYPE_REAL 0

struct Disk {
    PEACHOS_DISK_TYPE type;
    int sector_size;
};

void disk_search_and_init();

struct Disk* disk_get(int index);

int disk_read_block(struct Disk* iDisk, unsigned int lba, int total, void* buf);

#endif //LINUX_KERNEL_DISK_H
