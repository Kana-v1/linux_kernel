//
// Created by kana on 7/29/23.
//

#include "fat16.h"

int fat16_resolve(struct Disk* disk);
void* fat16_open(struct Disk* disk, struct PathPart* path, FILE_MODE mode);
int fat16_read(struct Disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);

struct Filesystem fat16_fs = {
        .resolve = fat16_resolve,
        .open = fat16_open,
		.read = fat16_read,
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
    fat_directory->end_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);

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

void fat16_to_proper_string(char** out, const char* in) {
    const int space = 0x20;
    const int null_terminator = 0x00;

    while (*in != null_terminator && *in != space) {
        **out = *in;
        *out += 1;
        in += 1;
    }

    if (*in == space) {
        **out = null_terminator;
    }
}

void fat16_get_full_relative_filename(struct FatDirectoryItem* item, char* out, int max_length) {
    memset(out, 0x00, max_length);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);

    const int space = 0x20;
    const int null_terminator = 0x00;
    if (item->ext[0] != null_terminator && item->ext[0] != space) {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }
}

struct FatDirectoryItem* fat16_clone_directory_item(struct FatDirectoryItem* item, int size) {
    if (size < sizeof(struct FatDirectoryItem)) {
        return 0;
    }

    struct FatDirectoryItem* item_copy = kzalloc(size);
    if (!item_copy) {
        return 0;
    }

    memcpy(item_copy, item, size);

    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct FatDirectoryItem* item) {
    return item->high_16_bits_first_cluster | item->low_16_bits_first_cluster;
}

static int fat16_cluster_to_sector(struct FatPrivate* fat_private, int cluster) {
    int num_of_clusters_to_ignore = 2;
    return fat_private->root_directory.end_sector_pos +
           ((cluster - num_of_clusters_to_ignore) * fat_private->header.primary_header.sectors_per_cluster);
}

static int fat16_get_first_fat_sector(struct FatPrivate* private) {
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct Disk* disk, int cluster) {
    struct FatPrivate* private = disk->fs_private;
    struct DiskStream* stream = private->fat_read_stream;

    if (!stream) {
        return EIO;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    int res = diskstreamer_seek(stream, fat_table_position * (cluster * PEACHOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0) {
        return res;
    }


    uint16_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0) {
        return res;
    }

    return result;
}

// get the correct cluster to use based on the starting cluster and the offset
static int fat16_get_cluster_for_offset(struct Disk* disk, int starting_cluster, int offset) {
    struct FatPrivate* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int cluster_ahead = offset / size_of_cluster_bytes;

    for (int i = 0; i < cluster_ahead; i++) {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF) {
            // we are at the last entry in the file
            return EIO;
        }

        if (entry == PEACHOS_FAT16_BAD_SECTOR) {
            return EIO;
        }

        // is sector reserved?
        if (entry == 0xFF0 || entry == 0xFF6) {
            return EIO;
        }

        // first 2 clusters are ignoresso it shouldn't be 0
        if (entry == 0x00) {
            return EIO;
        }

        cluster_to_use = entry;
    }

    return cluster_to_use;
}

static int fat16_read_internal_from_stream(struct Disk* disk, struct DiskStream* stream, int starting_cluster, int offset, int total, void* out) {
    int res = PEACHOS_ALL_OK;
    struct FatPrivate* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, starting_cluster, offset);
    if (cluster_to_use < 0) {
        return cluster_to_use;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;

    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != PEACHOS_ALL_OK) {
        return res;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != PEACHOS_ALL_OK) {
        return res;
    }

    total -= total_to_read;
    if (total > 0) {
        res = fat16_read_internal_from_stream(disk, stream, starting_cluster, offset + total_to_read, total, out + total_to_read);
    }

    return res;
}

static int fat16_read_internal(struct Disk* disk, int starting_cluster, int offset, int total, void* out) {
    struct FatPrivate* fs_private = disk->fs_private;
    struct DiskStream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_free_directory(struct FatDirectory* dir) {
    if (!dir) {
        return;
    }

    if (dir->item) {
        kfree(dir->item);
    }

    kfree(dir);
}

void fat16_fat_item_free(struct FatItem* item) {
    if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
        fat16_free_directory(item->directory);
    } else if (item->type == FAT_ITEM_TYPE_FILE) {
        kfree(item->item);
    }

    kfree(item);
}

struct FatDirectory* fat16_load_fat_directory(struct Disk* disk, struct FatDirectoryItem* item) {
    int res = 0;
    struct FatDirectory* dir = 0;
    struct FatPrivate* fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY)) {
        res = EINVARG;
        goto out;
    }

    dir = kzalloc(sizeof(struct FatDirectory));
    if (!dir) {
        res = ENOMEM;
        goto out;
    }

    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    dir->total = total_items;
    int dir_size = dir->total * sizeof(struct FatDirectoryItem);

    dir->item = kzalloc(dir_size);
    if (!dir->item) {
        res = ENOMEM;
        goto out;
    }

    res = fat16_read_internal(disk, cluster, 0x00, dir_size, dir->item);

    out:
    if (res != PEACHOS_ALL_OK) {
        fat16_free_directory(dir);
    }

    return dir;
}

struct FatItem* fat16_new_fat_item_for_directory_item(struct Disk* disk, struct FatDirectoryItem* item) {
    struct FatItem* f_item = kzalloc(sizeof(struct FatItem));
    if (!f_item) {
        return 0;
    }

    if (item->attribute & FAT_FILE_SUBDIRECTORY) {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item = fat16_clone_directory_item(item, sizeof(struct FatDirectoryItem));

    return f_item;
}

struct FatItem* fat16_find_item_in_dir(struct Disk* disk, struct FatDirectory* dir, const char* name) {
    struct FatItem* f_item = 0;
    char tmp_filename[PEACHOS_MAX_PATH];

    for (int i = 0; i < dir->total; i++) {
        fat16_get_full_relative_filename(&dir->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0) {
            f_item = fat16_new_fat_item_for_directory_item(disk, &dir->item[i]);
        }
    }

    return f_item;
}

struct FatItem* fat16_get_directory_entry(struct Disk* disk, struct PathPart* path) {
    struct FatPrivate* fat_private = disk->fs_private;
    struct FatItem* current_item = 0;
    struct FatItem* root_item = fat16_find_item_in_dir(disk, &fat_private->root_directory, path->part);

    if (!root_item) {
        return current_item;
    }

    struct PathPart* next_part = path->next;
    current_item = root_item;
    while (next_part != 0) {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = 0;
            break;
        }

        struct FatItem* tmp_item = fat16_find_item_in_dir(disk, current_item->directory, next_part->part);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }

    return current_item;
}

void* fat16_open(struct Disk* disk, struct PathPart* path, FILE_MODE mode) {
    if (mode != FILE_MODE_READ) {
        return ERROR(ERDONLY);
    }

    struct FatFileDescriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct FatFileDescriptor));
    if (!descriptor) {
        return ERROR(ENOMEM);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item) {
        return ERROR(EIO);
    }

    descriptor->pos = 0;
    return descriptor;
}

int fat16_read(struct Disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr) {
	int res = PEACHOS_ALL_OK;

	struct FatFileDescriptor* fat_desc = descriptor;
	struct FatDirectoryItem* item = fat_desc->item->item;
	int offset = fat_desc->pos;
	for (uint32_t i = 0; i < nmemb; i++) {
		res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
		if (IS_ERROR(res)) {
			return res;
		}

		out_ptr += size;
		offset += size;
	}

	res = nmemb;
	return res;
}
