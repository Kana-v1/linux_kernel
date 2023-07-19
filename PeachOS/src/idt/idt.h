// Created by kana on 7/13/23.
//

#ifndef LINUX_KERNEL_IDT_H
#define LINUX_KERNEL_IDT_H

#include <stdint.h>
#include "../kernel.h"

// interrupt descriptor table
struct idt_desc {
    uint16_t offset_1;  // offset bits 0-15
    uint16_t selector;  // selector that in out GDT
    uint8_t zero;       // does nothing, unused set to zero
    uint8_t type_attr;  // descriptor type and attributes
    uint16_t offset_2;  // offset bits 16-31
}__attribute__((packed));

// idt location
struct idtr_desc {
    uint16_t limit;     // size of descriptor table - 1
    uint32_t base;      // base of the start of the interrupt descriptor table
}__attribute__((packed));

void idt_init(void);
void enable_interrupts(void);
void disable_interrupts(void);

#endif //LINUX_KERNEL_IDT_H
