//
// Created by kana on 7/22/23.
//

#ifndef LINUX_KERNEL_PPARSER_H
#define LINUX_KERNEL_PPARSER_H

#include "../kernel.h"
#include "../string/string.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../status.h"

struct PathRoot {
    int drive_no;
    struct PathPart* first;
};

struct PathPart {
    const char* part;
    struct PathPart* next;
};

struct PathRoot* pathparser_parse(const char* path, const char* current_directory_path);
void pathparser_free(struct PathRoot* root);

#endif //LINUX_KERNEL_PPARSER_H
