#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <interrupts.h>

#define SYSCALLS_IDT_ENTRY 49
#define SYSCALLS_IRQ_ENTRY SYSCALLS_IDT_ENTRY - 32

typedef enum 
{
    SYSCALL_PUTS, 
    SYSCALL_PUTI,
    SYSCALL_EXIT,
    SYSCALL_FILE_FIND,
    SYSCALL_FILE_READ,
    SYSCALL_FILE_DIRSIZE,
    SYSCALL_FILE_LS,

    SYSCALL_EXEC,

    SYSCALL_VBUFF,
    SYSCALL_DISPOSEVBUFF,

    SYSCALL_HAND_ATTACH_MOUSE,
    SYSCALL_HAND_ATTACH_KB,
    SYSCALL_HAND_REMOVE,
} syscalls_t;

#define STR1(x)  #x
#define STR(x)  STR1(x)

#define SYSCALL_PARAM_1 esi
#define SYSCALL_PARAM_2 edi
#define SYSCALL_PARAM_3 edx
#define SYSCALL_PARAM_4 ecx

#define SYSCALL_PARAM_1_ "S"  // esi
#define SYSCALL_PARAM_2_ "D"  // edi
#define SYSCALL_PARAM_3_ "d"  // edx
#define SYSCALL_PARAM_4_ "c"  // edx

#define SYSCALL(num, data)                              __asm__ volatile ("int $" STR(SYSCALLS_IDT_ENTRY) : : "a"(num), SYSCALL_PARAM_1_ (data) : "memory")
#define SYSCALL_2PARAM(num, data1, data2)               __asm__ volatile ("int $" STR(SYSCALLS_IDT_ENTRY) : : "a"(num), SYSCALL_PARAM_1_ (data1),  SYSCALL_PARAM_2_ (data2): "memory")
#define SYSCALL_3PARAM(num, data1, data2, data3)        __asm__ volatile ("int $" STR(SYSCALLS_IDT_ENTRY) : : "a"(num), SYSCALL_PARAM_1_ (data1), SYSCALL_PARAM_2_ (data2), SYSCALL_PARAM_3_ (data3) : "memory")
#define SYSCALL_4PARAM(num, data1, data2, data3, data4) __asm__ volatile ("int $" STR(SYSCALLS_IDT_ENTRY) : : "a"(num), SYSCALL_PARAM_1_ (data1), SYSCALL_PARAM_2_ (data2), SYSCALL_PARAM_3_ (data3), SYSCALL_PARAM_4_ (data4) : "memory")

void syscall_create(interrupt_fn_t fn, uint32_t index);
void syscalls_init();

extern void syscall_stub(void);

#endif