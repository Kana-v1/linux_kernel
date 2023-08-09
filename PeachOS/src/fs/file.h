//
// Created by kana on 7/23/23.
//

#ifndef LINUX_KERNEL_FILE_H
#define LINUX_KERNEL_FILE_H

#include "pparser.h"
#include "../config.h"

typedef unsigned int FILE_SEEK_MODE;
enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};


struct Disk;
struct PathPart;

typedef void* (*FS_OPEN_FUNCTION)(struct Disk* disk, struct PathPart* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(struct Disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_RESOLVE_FUNCTION)(struct Disk* disk);

struct Filesystem {
    // filesystem should return 0 from resolve if the provider disk is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
	FS_READ_FUNCTION read;

    char name[20];
};

struct FileDescriptor {
    int index; // the descriptor index
    struct Filesystem* filesystem;

    void* private; // private data for internal file descriptor

    struct Disk* disk; // the disk that the file descriptor should be used on
};

void fs_init();

int fopen(const char* filename, const char* mode_string);

int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);

void fs_insert_filesystem(struct Filesystem* filesystem);

struct Filesystem* fs_resolve(struct Disk* disk);

#endif //LINUX_KERNEL_FILE_H



