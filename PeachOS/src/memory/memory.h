//
// Created by kana on 7/13/23.
//

#ifndef LINUX_KERNEL_MEMORY_H
#define LINUX_KERNEL_MEMORY_H

#include <stddef.h>

void* memset(void* ptr, int c, size_t size);

int memcmp(void* s1, void* s2, int count);

void* memcpy(void* dest, void* src, int len);


#endif //LINUX_KERNEL_MEMORY_H
