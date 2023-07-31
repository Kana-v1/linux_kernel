//
// Created by kana on 7/22/23.
//

#ifndef LINUX_KERNEL_STRING_H
#define LINUX_KERNEL_STRING_H

#include <stdbool.h>

int strlen(const char* ptr);

int strnlen(const char* ptr, int max);

bool is_digit(char c);

int to_numeric_digit(char c);

char* strcpy(char* dest, const char* src);

#endif //LINUX_KERNEL_STRING_H
