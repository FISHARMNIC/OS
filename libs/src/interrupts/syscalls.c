#include <syscalls.h>
#include <graphics.h>
#include <cpu.h>

static interrupt_fn_t syscalls[256];

void syscall_create(interrupt_fn_t fn, uint32_t index)
{
    syscalls[index] = fn;
}

static void _syscall_puts(regs32_t registers)
{
    // esi: string
    tty_printf("SYS PUTS CALLED: %s\n", (char*) registers.esi);
}

static void _syscall_puti(regs32_t registers)
{
    // esi: number
    tty_printf("SYS PUTI CALLED: %d\n", registers.esi);
}

static void _syscall_exit(regs32_t registers)
{
    // esi: return value (+1 since 0 is setjmp return)
    longjmp(kernel_return_ctx, registers.eip + 1);
}

void syscall_dispatch(regs32_t r)
{
    tty_printf("[SYSCALL] called! eax=%d\n", r.eax);
    (syscalls[r.eax])(r);
}

void syscalls_init()
{
    syscall_create(_syscall_puts, SYSCALL_PUTS);
    syscall_create(_syscall_puti, SYSCALL_PUTI);
    syscall_create(_syscall_exit, SYSCALL_EXIT);
    
    idt_set_gate_user(SYSCALLS_IDT_ENTRY, (void*) syscall_stub);
}