//
// Created by kana on 7/21/23.
//

#include "disk.h"

struct Disk disk; // a primary hard disk

int disk_read_sector(int lba, int total, void* buf) {
    // here we just instruct disk controller that we want to read from it
    // https://wiki.osdev.org/ATA_Command_Matrix
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char) (lba & 0xff));
    outb(0x1F4, (unsigned char) (lba >> 8));
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buf;
    for (int i = 0; i < total; i++) {
        // wait for the buffer to be ready
        char c = insb(0x1F7);

        while (!(c & 0x08)) { // clear flags and wait till data is set
            c = insb(0x1F7);
        }

        // copy from hard disk to memory
        for (int i = 0; i < 256; i++) {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }

    return PEACHOS_ALL_OK;
}

void disk_search_and_init() {
    memset(&disk, 0, sizeof(disk));
    disk.type = PEACH_OS_DISK_TYPE_REAL;
    disk.sector_size = PEACHOS_SECTOR_SIZE;
}

struct Disk* disk_get(int index) {
    if (index != 0) {
        return 0;
    }

    return &disk;
}

int disk_read_block(struct Disk* iDisk, unsigned int lba, int total, void* buf) {
    if (iDisk != &disk) {
        return EIO;
    }

    return disk_read_sector(lba, total, buf);
}
