#ifndef USER_MALLOC_H
#define USER_MALLOC_H

#include <syscalls.h>
#include <stdint.h>

static inline void* malloc(uint32_t size)
{
    void *alloc = NULLPTR;

    SYSCALL_2PARAM(SYSCALL_MALLOC, size, &alloc);

    return alloc;
}

static inline void free(void *ptr)
{
    SYSCALL(SYSCALL_FREE, ptr);
}

#endif