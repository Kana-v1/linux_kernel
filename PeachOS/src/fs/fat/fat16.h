//
// Created by kana on 7/29/23.
//

#ifndef LINUX_KERNEL_FAT16_H
#define LINUX_KERNEL_FAT16_Hasd

#include "../file.h"
#include "../../string/string.h"

#define PEACHOS_FAT16_SIGNATURE         0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE    0x02
#define PEACHOS_FAT16_BAD_SECTOR        0xFF7
#define PEACHOS_FAT16_UNUSED            0xFF7

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY     0
#define FAT_ITEM_TYPE_FILE          1

// fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY      0x01
#define FAT_FILE_HIDDEN         0x02
#define FAT_FILE_SYSTEM         0x04
#define FAT_FILE_VOLUME_LABEL   0x08
#define FAT_FILE_SUBDIRECTORY   0x10
#define FAT_FILE_ARCHIVED       0x20
#define FAT_FILE_DEVICE         0x40
#define FAT_FILE_RESERVED       0x80


struct FatHeaderExtended {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed)); // we're writing and reading this to the disk so compiler should not shuffle the order

struct FatHeader {
    uint8_t short_jmp_ins[3]; // short jump instruction
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct FatH {
    struct FatHeader primary_header;
    union fat_h_e {
        struct FatHeaderExtended extended_header;
    } shared;
};

struct FatDirectoryItem {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct FatDirectory {
    struct FatDirectoryItem* item;
    int total;      // total number of items in the fat directory
    int sector_pos; // where the fat directory is
    int end_sector_pos; // the last sector of the where this directory is stored
};

struct FatItem {
    union {
        struct FatDirectoryItem* item;
        struct FatDirectory* directory;
    };

    FAT_ITEM_TYPE type;
};

struct FatFileDescriptor {
    struct FatItem* item;
    uint32_t pos;
};

struct FatPrivate {
    struct FatH header;
    struct FatDirectory root_directory;

    struct DiskStream* cluster_read_stream; // used to stream data clusters
    struct DiskStream* fat_read_stream;     // used to stream the file allocation table
    struct DiskStream* directory_stream;    // used to stream the directory
};

struct Filesystem* fat16_init();

#endif //LINUX_KERNEL_FAT16_H
