#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <os_setjmp.h>

typedef struct
{
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    uint32_t intn, errc;
    uint32_t eip, cs, eflags, usresp, ss; 
} regs32_t;

#define NULLPTR ((void*)0)


#endif