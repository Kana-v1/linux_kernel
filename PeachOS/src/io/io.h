//
// Created by kana on 7/15/23.
//

#ifndef LINUX_KERNEL_IO_H
#define LINUX_KERNEL_IO_H

unsigned char insb(unsigned short port);    // input 1 byte
unsigned short insw(unsigned short port);   // input 2 bytes

void outb(unsigned short port, unsigned char val);  // output 1 byte
void outw(unsigned short port, unsigned short val); // output 2 bytes

#endif //LINUX_KERNEL_IO_H
