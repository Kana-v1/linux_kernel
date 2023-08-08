//
// Created by kana on 7/17/23.
//

#ifndef LINUX_KERNEL_KHEAP_H
#define LINUX_KERNEL_KHEAP_H

#include <stdint.h>
#include <stddef.h>

void kheap_init(void);

void* kmalloc(size_t size);

void kfree(void* ptr);

void* kzalloc(size_t size);


#endif //LINUX_KERNEL_KHEAP_H
