//
// Created by kana on 7/23/23.
//

#include "file.h"
#include "fat/fat16.h"

struct Filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct FileDescriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

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
        panic("Problem inserting filesystem");
    }

    *fs = filesystem;
}

static void fs_static_load() {
    fs_insert_filesystem(fat16_init());
}

void fs_load() {
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static void file_free_descriptor(struct FileDescriptor* desc) {
	file_descriptors[desc->index - 1] = 0x00;
	kfree(desc);
}

static int file_new_descriptor(struct FileDescriptor** descriptor_out) {
    int res = ENOMEM;
    for (int i = 0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++) {
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
    if (fd_index <= 0 || fd_index >= PEACHOS_MAX_FILE_DESCRIPTORS) {
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

FILE_MODE file_get_mode_by_string(const char* str) {
    FILE_MODE mode = FILE_MODE_INVALID;

    if (strncmp(str, "r", 1) == 0) {
        mode = FILE_MODE_READ;
    } else if (strncmp(str, "w", 1) == 0) {
        mode = FILE_MODE_WRITE;
    } else if (strncmp(str, "a", 1) == 0) {
        mode = FILE_MODE_APPEND;
    }

    return mode;
}

int fopen(const char* filename, const char* mode_string) {
    const int fopen_error = 0;
    struct PathRoot* root_path = pathparser_parse(filename, NULL);
    if (!root_path) {
        return fopen_error;
    }

    // we have to have a full path (0:/text.txt, not 0:/)
    if (!root_path->first) {
        return fopen_error;
    }

    struct Disk* disk = disk_get(root_path->drive_no);
    if (!disk) {
        return fopen_error;
    }

    if (!disk->filesystem) {
        return fopen_error;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_string);
    if (mode == FILE_MODE_INVALID) {
        return fopen_error;
    }

    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);

    if (IS_ERROR(descriptor_private_data)) {
        return fopen_error;
    }

    struct FileDescriptor* desc = 0;
    int res = file_new_descriptor(&desc);
    if (res < 0) {
        return fopen_error;
    }

    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence) {
	struct FileDescriptor* desc = file_get_descriptor(fd);
	if (!desc) {
		return EIO;
	}

	return desc->filesystem->seek(desc->private, offset, whence);
}

int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd) {
	if (size == 0 || nmemb == 0 || fd < 1) {
		return EINVARG;
	}

	struct FileDescriptor* desc = file_get_descriptor(fd);
	if (!desc) {
		return EINVARG;
	}

	return desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char*)ptr);
}

int fstat(int fd, struct FileStat* stat) {
	struct FileDescriptor* desc = file_get_descriptor(fd);
	if (!desc) {
		return EIO;
	}

	return desc->filesystem->stat(desc->disk, desc->private, stat);
}

int fclose(int fd) {
	struct FileDescriptor* desc = file_get_descriptor(fd);
	if (!desc) {
		return EIO;
	}

	int res = desc->filesystem->close(desc->private);
	if (res == PEACHOS_ALL_OK) {
		file_free_descriptor(desc);
	}

	return PEACHOS_ALL_OK;
}
