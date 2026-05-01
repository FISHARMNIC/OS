#include <syscalls.h>
#include <graphics.h>
#include <cpu.h>
#include <files.h>
#include <elf.h>
#include <sys/kmalloc.h>
#include <paging.h>
#include <events.h>

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

// @todo
// static void _syscall_file_dirsize(regs32_t registers)
// {
// }

// @todo should just be simpler, pass directory only and should somehow setjmp here

static void _syscall_exec(regs32_t registers) // @todo fix, page faults
{
    fd_t *fd = (fd_t *)registers.SYSCALL_PARAM_1;
    uint32_t argc = (uint32_t)registers.SYSCALL_PARAM_2;
    char **argv = (char **)registers.SYSCALL_PARAM_3;
    uint32_t *resp = (uint32_t *)registers.SYSCALL_PARAM_4;

    uint32_t size = file_size(fd);
    // uint8_t buffer[size]; // @todo kmalloc AND MAKE SURE FREE -> maybe needs to be done in elf part
    uint8_t *buffer = kmalloc(size);

    if (buffer == NULLPTR)
    {
        tty_printf("[ERROR] Malloc Failiure\n");
        return;
    }

    // tty_printf("File size %d\n", size);

    *resp = file_read(fd, buffer, size);
    if (resp == 0)
    {
        return;
    }

    // static uint8_t ustack[user_stack_size] __attribute__((aligned(user_stack_size)));

    exec_pending_file = buffer;
    exec_pending_size = size;
    exec_pending_argc = argc;
    exec_pending_argv = argv;
    exec_pending = 1;
    exec_free_buffer = true;

    // exit current process — longjmp back to elf_exec's setjmp
    longjmp(kernel_return_ctx, 1);

    // *resp = elf_exec(buffer, size, user_stack_glob, user_stack_size, argc, argv);
}

static void _syscall_getvbuff(regs32_t registers)
{
    framebuffer_t *info = (framebuffer_t *)registers.SYSCALL_PARAM_1;

    paging_set_user_range(
        graphics_fb_active->addr,
        graphics_fb_active->pitch * graphics_fb_active->height);

    *info = *graphics_fb_active;
}

static void _syscall_dispvbuff(regs32_t registers)
{
    framebuffer_t *info = (framebuffer_t *)registers.SYSCALL_PARAM_1;

    paging_clear_user_range(
        info->addr,
        info->pitch * info->height);
}

static void _syscall_attachints_mouse(regs32_t registers)
{
    event_on_click_fn click = (event_on_click_fn)registers.SYSCALL_PARAM_1;
    event_on_move_fn move = (event_on_move_fn)registers.SYSCALL_PARAM_2;

    int32_t *resp1 = (int32_t *)registers.SYSCALL_PARAM_3;
    int32_t *resp2 = (int32_t *)registers.SYSCALL_PARAM_4;

    *resp1 = event_click_add_handler(click);
    *resp2 = event_move_add_handler(move);
}

static void _syscall_attachints_keyboard(regs32_t registers)
{
    event_on_key_fn key = (event_on_key_fn)registers.SYSCALL_PARAM_1;

    int32_t *resp = (int32_t *)registers.SYSCALL_PARAM_2;

    *resp = event_keyboard_add_handler(key);
}

static void _syscall_removints(regs32_t registers)
{
    int32_t click = (int32_t)registers.SYSCALL_PARAM_1;
    int32_t move = (int32_t)registers.SYSCALL_PARAM_2;
    int32_t key = (int32_t)registers.SYSCALL_PARAM_3;

    event_click_remove_handler(click);
    event_move_remove_handler(move);
    event_keyboard_remove_handler(key);
}

// @todo remove interrupts

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

    syscall_create(_syscall_exec, SYSCALL_EXEC);

    syscall_create(_syscall_getvbuff, SYSCALL_VBUFF);
    syscall_create(_syscall_dispvbuff, SYSCALL_DISPOSEVBUFF);

    syscall_create(_syscall_attachints_mouse, SYSCALL_HAND_ATTACH_MOUSE);
    syscall_create(_syscall_attachints_keyboard, SYSCALL_HAND_ATTACH_KB);
    syscall_create(_syscall_removints, SYSCALL_HAND_REMOVE);

    idt_set_gate_user(SYSCALLS_IDT_ENTRY, (void *)syscall_stub);
}