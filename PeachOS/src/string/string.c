//
// Created by kana on 7/22/23.
//

#include "string.h"

const static int ascii0 = 48;
const static int ascii9 = 57;

int strlen(const char* ptr) {
    int i = 0;
    while (*ptr != 0) {
        i++;
        ptr++;
    }

    return i;
}

int strnlen(const char* ptr, int max) {
    int i = 0;
    for (; i < max; i++) {
        if (ptr[i] == 0) {
            break;
        }
    }

    return i;
}

char* strcpy(char* dest, const char* src) {
    char* res = dest;

    while (*src != 0) {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00;

    return res;
}

bool is_digit(char c) {
    return c >= ascii0 && c <= ascii9;
}

int to_numeric_digit(char c) {
    return c - ascii0;
}