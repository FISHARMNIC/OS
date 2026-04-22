#include <sys/string.h>

uint32_t strlen(const char* str)
{
    uint32_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

uint32_t strcmp(const char* s1, const char* s2)
{
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return *s1 - *s2;
}