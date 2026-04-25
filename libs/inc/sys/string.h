#ifndef STRING_H
#define STRING_H

#include <stdint.h>

uint32_t strlen(const char* str);
uint32_t strcmp(const char* s1, const char* s2);

void* memcpy(void* dest, const void* src, uint32_t size);
void* memset(void* dest, const uint8_t src, uint32_t size);
char *strchr(const char *s, char c);
char *strtok_r(char *str, const char *delim, char **saveptr);
void str_toupper(char *s);

#endif