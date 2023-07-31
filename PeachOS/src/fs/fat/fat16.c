//
// Created by kana on 7/29/23.
//

#include "fat16.h"

int fat16_resolve(struct Disk* disk);

void* fat16_open(struct Disk* disk, struct PathPart* path, FILE_MODE mode);

struct Filesystem fat16_fs = {
        .resolve = fat16_resolve,
        .open = fat16_open
};

struct Filesystem* fat16_init() {
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

static void fat16_init_private(struct Disk* disk, struct FatPrivate* fat_private) {
    memset(fat_private, 0, sizeof(struct FatPrivate));
    fat_private->cluster_read_stream = diskstreamer_new(disk->id);
    fat_private->fat_read_stream = diskstreamer_new(disk->id);
    fat_private->directory_stream = diskstreamer_new(disk->id);
}

int fat16_sector_to_absolute(struct Disk* disk, int sector) {
    return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct Disk* disk, uint32_t dir_start_sector) {
    struct FatDirectoryItem item;
    struct FatDirectoryItem empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct FatPrivate* fat_private = disk->fs_private;

    int i = 0;
    int directory_start_pos = dir_start_sector * disk->sector_size;
    struct DiskStream* stream = fat_private->directory_stream;

    if (diskstreamer_seek(stream, directory_start_pos) != PEACHOS_ALL_OK) {
        return EIO;
    }

    const uint8_t directory_entry_is_free = 0xE5;

    while (1) {
        if (diskstreamer_read(stream, &item, sizeof(item)) != PEACHOS_ALL_OK) {
            return EIO;
        }

        if (item.filename[0] == 0x00) {
            break;
        }

        if (item.filename[0] == directory_entry_is_free) {
            continue;
        }

        i++;
    }

    return i;
}

int fat16_get_root_directory(struct Disk* disk, struct FatPrivate* fat_private, struct FatDirectory* fat_directory) {
    int res = PEACHOS_ALL_OK;

    struct FatHeader* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct FatDirectoryItem));
    int total_sectors = root_dir_size / disk->sector_size;
    if (root_dir_size % disk->sector_size) {
        total_sectors += 1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);
    struct FatDirectoryItem* dir = kzalloc(root_dir_size);
    if (!dir) {
        return ENOMEM;
    }

    struct DiskStream* stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != PEACHOS_ALL_OK) {
        return EIO;
    }

    if (diskstreamer_read(stream, dir, root_dir_size) != PEACHOS_ALL_OK) {
        return EIO;
    }

    fat_directory->item = dir;
    fat_directory->total = total_items;
    fat_directory->sector_pos = root_dir_sector_pos;
    fat_directory->end_sector = root_dir_sector_pos + (root_dir_size / disk->sector_size);

    return res;
}

int fat16_resolve(struct Disk* disk) {
    int res = PEACHOS_ALL_OK;

    struct FatPrivate* fat_private = kzalloc(sizeof(struct FatPrivate));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    struct DiskStream* stream = diskstreamer_new(disk->id);
    if (!stream) {
        res = ENOMEM;
        goto out;
    }

    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_ALL_OK) {
        res = EIO;
        goto out;
    }

    const uint8_t fat16_valid_signature = 0x29; // not 0x29 means it's not the fat16 filesystem

    if (fat_private->header.shared.extended_header.signature != fat16_valid_signature) {
        res = EFSNOTUS;
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_ALL_OK) {
        res = EIO;
        goto out;
    }

    out:
    if (stream) {
        diskstreamer_close(stream);
    }

    if (res < 0) {
        kfree(fat_private);
        disk->fs_private = 0;
    }

    return res;
}

void* fat16_open(struct Disk* disk, struct PathPart* path, FILE_MODE mode) {
    return 0;
}
