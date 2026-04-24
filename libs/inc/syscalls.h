#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <interrupts.h>

#define SYSCALLS_IDT_ENTRY 49
#define SYSCALLS_IRQ_ENTRY SYSCALLS_IDT_ENTRY - 32

#define SYSCALL_PUTS 0
#define SYSCALL_PUTI 1

#define STR1(x)  #x
#define STR(x)  STR1(x)

#define SYSCALL(num, data) __asm__ volatile("mov %0, %%eax; mov %1, %%esi;int $"STR(SYSCALLS_IDT_ENTRY)";" : : "r"(num), "r"((data)) : "eax", "esi")

void syscall_create(interrupt_fn_t fn, uint32_t index);
void syscalls_init();

extern void syscall_stub(void);

#endif