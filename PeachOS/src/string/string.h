//
// Created by kana on 7/22/23.
//

#ifndef LINUX_KERNEL_STRING_H
#define LINUX_KERNEL_STRING_H

#include <stdbool.h>

char to_lower(char s1);

int strlen(const char* ptr);

int strnlen(const char* ptr, int max);

bool is_digit(char c);

int to_numeric_digit(char c);

int strncmp(const char* str1, const char* str2, int n);

int strnlen_terminator(const char* str, int max, char terminator);

int istrncmp(const char* s1, const char* s2, int n);

char* strcpy(char* dest, const char* src);

char* strncpy(char* dest, const char* src, int n);

#endif //LINUX_KERNEL_STRING_H
