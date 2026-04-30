#include <stddef.h>

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

int strncmp(const char* a, const char* b, size_t n) {
    while (n-- && *a && (*a == *b)) {
        a++; b++;
    }
    return n == (size_t)-1 ? 0 : *(const unsigned char*)a - *(const unsigned char*)b;
}

size_t strlen(const char* s) {
    const char* p = s;
    while (*p) p++;
    return (size_t)(p - s);
}