#include <syscalls.h>
#include <graphics.h>
#include <cpu.h>
#include <files.h>
#include <elf.h>

static interrupt_fn_t syscalls[256];

void syscall_create(interrupt_fn_t fn, uint32_t index)
{
    syscalls[index] = fn;
}

static void _syscall_puts(regs32_t registers)
{
    tty_printf("%s", (char *)registers.SYSCALL_PARAM_1);
}

static void _syscall_puti(regs32_t registers)
{
    tty_printf("%d", registers.SYSCALL_PARAM_1);
}

static void _syscall_exit(regs32_t registers)
{
    // esi: return value (+1 since 0 is setjmp return)
    longjmp(kernel_return_ctx, registers.SYSCALL_PARAM_1 + 1);
}

static void _syscall_file_find(regs32_t registers)
{
    fd_t *fd = (fd_t *)registers.SYSCALL_PARAM_1;
    char *name = (char *)registers.SYSCALL_PARAM_2;
    uint32_t *resp = (uint32_t *)registers.SYSCALL_PARAM_3;

    // tty_printf("(KERN) fd at: %d\n", fd);

    // tty_printf("LOADING IN POINTERS %d %d %d\n", fd, name, resp);

    *resp = file_find(fd, name);

    // tty_printf("RESP %s %d\n", fd->name.name, fd->entry.fileSizeBytes);
}

static void _syscall_file_read(regs32_t registers)
{
    fd_t *fd = (fd_t *)registers.SYSCALL_PARAM_1;
    uint8_t *buffer = (uint8_t *)registers.SYSCALL_PARAM_2;
    uint32_t size = (uint32_t)registers.SYSCALL_PARAM_3;
    uint32_t *resp = (uint32_t *)registers.SYSCALL_PARAM_4;

    *resp = file_read(fd, buffer, size);

    // tty_printf("READ %d BYTES %s\n", size, buffer);
}

static void _syscall_file_ls(regs32_t registers)
{
    fd_t *fd_arr = (fd_t *)registers.SYSCALL_PARAM_1;
    uint32_t size = (uint32_t)registers.SYSCALL_PARAM_2;
    fd_t *fd = (fd_t *)registers.SYSCALL_PARAM_3;
    uint32_t *resp = (uint32_t *)registers.SYSCALL_PARAM_4;

    // tty_printf("\tLS SYSCALL, CLUSTER %d\n", fd->cluster);
    *resp = files_ls(fd_arr, size, fd == NULLPTR ? 0 : fd->cluster);

    // tty_printf("READ %d BYTES %s\n", size, buffer);
}

// @todo should just be simpler, pass directory only and should somehow setjmp here

// static void _syscall_spawn(regs32_t registers)
// {
//     fd_t* fd = (fd_t*) registers.SYSCALL_PARAM_1;
//     uint8_t* buffer =  (uint8_t*) registers.SYSCALL_PARAM_2;
//     uint32_t size =  (uint32_t) registers.SYSCALL_PARAM_3;
//     uint32_t* resp =  (uint32_t*) registers.SYSCALL_PARAM_4;

//     *resp = file_read(fd, buffer, size);
//     if(resp == 0)
//     {
//         return;
//     }

//     elf_exec(buffer, )
// }


void syscall_dispatch(regs32_t r)
{
    // tty_printf("[SYSCALL] called! eax=%d\n", r.eax);
    (syscalls[r.eax])(r);
}

void syscalls_init()
{
    syscall_create(_syscall_puts, SYSCALL_PUTS);
    syscall_create(_syscall_puti, SYSCALL_PUTI);
    syscall_create(_syscall_exit, SYSCALL_EXIT);

    syscall_create(_syscall_file_find, SYSCALL_FILE_FIND);
    syscall_create(_syscall_file_read, SYSCALL_FILE_READ);
    syscall_create(_syscall_file_ls, SYSCALL_FILE_LS);

    idt_set_gate_user(SYSCALLS_IDT_ENTRY, (void *)syscall_stub);
}