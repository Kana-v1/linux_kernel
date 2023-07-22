//
// Created by kana on 7/22/23.
//

#ifndef LINUX_KERNEL_STREAMER_H
#define LINUX_KERNEL_STREAMER_H

#include "disk.h"
#include "../memory/heap/kheap.h"

struct DiskStream {
    int pos;            // position we currently at in the stream
    struct Disk* disk;
};

struct DiskStream* diskstreamer_new(int disk_id);

int diskstreamer_seek(struct DiskStream* stream, int pos);

int diskstreamer_read(struct DiskStream* stream, void* out, int total);

void diskstreamer_close(struct DiskStream* stream);

#endif //LINUX_KERNEL_STREAMER_H
