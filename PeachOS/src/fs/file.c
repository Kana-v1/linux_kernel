//
// Created by kana on 7/23/23.
//

#include "file.h"

struct Filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct FileDescriptor* file_descriptors[PEACHOS_MAX_FILEDESCRIPTORS];

static struct Filesystem** fs_get_free_filesystem() {
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] == 0) {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct Filesystem* filesystem) {
    struct Filesystem** fs;
    fs = fs_get_free_filesystem();

    if (!fs) {
        print("\nProblem inserting filesystem");
        while (1) {} // panic is not implemented yet
    }

    *fs = filesystem;
}

static void fs_static_load() {
//    fs_insert_filesystem(fat16_init());
}

void fs_load() {
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct FileDescriptor** descriptor_out) {
    int res = ENOMEM;
    for (int i = 0; i < PEACHOS_MAX_FILEDESCRIPTORS; i++) {
        if (file_descriptors[i] == 0) {
            struct FileDescriptor* desc = kzalloc(sizeof(struct FileDescriptor));
            // descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *descriptor_out = desc;
            res = PEACHOS_ALL_OK;
            break;
        }
    }

    return res;
}

static struct FileDescriptor* file_get_descriptor(int fd_index) {
    if (fd_index <= 0 || fd_index >= PEACHOS_MAX_FILEDESCRIPTORS) {
        return 0;
    }

    // descriptors start at the index 1
    int index = fd_index - 1;

    return file_descriptors[index];
}

struct Filesystem* fs_resolve(struct Disk* disk) {
    struct Filesystem* fs = 0;
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == PEACHOS_ALL_OK) {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}

int fopen(const char* filename, const char mode) {
    return EIO;
}
