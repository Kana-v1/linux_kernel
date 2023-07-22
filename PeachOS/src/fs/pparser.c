//
// Created by kana on 7/22/23.
//

#include "pparser.h"

static int pathparser_path_valid_format(const char* fileName) {
    int len = strnlen(fileName, PEACHOS_MAX_PATH);

    return (len >= 3 && is_digit(fileName[0]) && memcmp((void*) &fileName[1], ":/", 2) == 0);
}

static int pathparser_get_drive_by_path(const char** path) {
    if (!pathparser_path_valid_format(*path)) {
        return EBADPATH;
    }

    int drive_no = to_numeric_digit(*path[0]);

    // add 3 bytes to skip drive number 0:/ 1:/
    *path += 3;

    return drive_no;
}

static struct PathRoot* path_parser_create_root(int drive_number) {
    struct PathRoot* path_r = kzalloc(sizeof(struct PathRoot));
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

static const char* pathparser_get_path_part(const char** path) {
    char* result_path_part = kzalloc(PEACHOS_MAX_PATH);
    int i = 0;
    while (**path != '/' && **path != 0x00) {
        result_path_part[i] = **path;
        *path += 1;
        i++;
    }

    if (**path == '/') {
        *path += 1; // skip the forward slash to avoid problems
    }

    if (i == 0) {
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part;
}

struct PathPart* pathparser_parse_path_part(struct PathPart* last_part, const char** path) {
    const char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str) {
        return 0;
    }

    struct PathPart* part = kzalloc(sizeof(struct PathPart));
    part->part = path_part_str;
    part->next = 0x00;

    if (last_part) {
        last_part->next = part;
    }

    return part;
}

void pathparser_free(struct PathRoot* root) {
    struct PathPart* part = root->first;

    while (part) {
        struct PathPart* next_part = part->next;
        kfree((void*) part->part);
        kfree(part);
        part = next_part;
    }

    kfree(root);
}

struct PathRoot* pathparser_parse(const char* path, const char* current_directory_path) {
    int res = 0;
    const char* tmp_path = path;
    struct PathRoot* path_root = 0;

    if (strlen(path) > PEACHOS_MAX_PATH) {
        return path_root;
    }

    res = pathparser_get_drive_by_path(&tmp_path);
    if (res < 0) {
        return path_root;
    }

    path_root = path_parser_create_root(res);
    if (!path_root) {
        return path_root;
    }

    struct PathPart* part = pathparser_parse_path_part(NULL, &tmp_path);
    if (!part) {
        return path_root;
    }

    path_root->first = part;
//    struct PathPart* part = pathparser_parse_path_part(first_part, &tmp_path);
    while (part) {
        part = pathparser_parse_path_part(part, &tmp_path);
    }

    return path_root;
}



