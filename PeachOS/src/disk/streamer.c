//
// Created by kana on 7/22/23.
//

#include "streamer.h"

struct DiskStream* diskstreamer_new(int disk_id) {
    struct Disk* disk = disk_get(disk_id);
    if (!disk) {
        return 0;
    }

    struct DiskStream* streamer = kzalloc(sizeof(struct DiskStream));
    streamer->pos = 0;
    streamer->disk = disk;

    return streamer;
}

int diskstreamer_seek(struct DiskStream* stream, int pos) {
    stream->pos = pos;
    return PEACHOS_ALL_OK;
}

int diskstreamer_read(struct DiskStream* stream, void* out, int total) {
    int sector = stream->pos / PEACHOS_SECTOR_SIZE;
    int offset = stream->pos % PEACHOS_SECTOR_SIZE;
    char buf[PEACHOS_SECTOR_SIZE];

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0) {
        return res;
    }

    int total_to_read = total > PEACHOS_SECTOR_SIZE ? PEACHOS_SECTOR_SIZE : total;
    for (int i = 0; i < total_to_read; i++) {
        *(char*)out++ = buf[offset + i];
    }

    stream->pos += total_to_read;
    if (total > PEACHOS_SECTOR_SIZE) {
        res = diskstreamer_read(stream, out, total - PEACHOS_SECTOR_SIZE);
    }

    return res;
}

void diskstreamer_close(struct DiskStream* stream) {
    kfree(stream);
}