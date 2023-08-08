//
// Created by kana on 7/22/23.
//

#include "string.h"

const static int ascii0 = 48;
const static int ascii9 = 57;

char to_lower(char s1) {
    int ascii_A = 65;
    int ascii_Z = 90;
    if (s1 > ascii_A && s1 <= ascii_Z) {
        s1 += 32;
    }

    return s1;
}

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

int strncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;

    while(n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) {
            return u1 - u2;
        }

        if (u1 == '\0') {
            return 0;
        }
    }

    return 0;
}

int istrncmp(const char* s1, const char* s2, int n) {
    unsigned char u1, u2;
    while(n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && to_lower(u1) != to_lower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

int strnlen_terminator(const char* str, int max, char terminator) {
    int i = 0;

    for (; i < max; i++) {
        if (str[i] == '\0' || str[i] == terminator) {
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