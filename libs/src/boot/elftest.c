#include <elf.h>
#include <graphics.h>
#include <os_setjmp.h>
#include <files.h>

#define user_stack_size 4096
uint8_t user_stack[user_stack_size] __attribute__((aligned(user_stack_size)));

// #include "../../userspace/gen/inc/test.h"

void elftest(iret_return_fn_t ret)
{
    (void)ret;
    tty_clear();
    // elf_exec(_Users_nico_Documents_OS_userspace_util____gen_bin_test_elf, _Users_nico_Documents_OS_userspace_util____gen_bin_test_elf_len, user_stack, user_stack_size, ret);
    

    fd_t fd;
    uint32_t err = file_find(&fd, "TEST/TEST.ELF");
    
    if(err)
    {
        tty_puts("[ERROR] File not found\n");
    }
    else
    {
        tty_puts("Found file\n");

        uint32_t size = file_size(&fd);
        uint8_t buffer[size];

        file_read(&fd, buffer, size);

        tty_printf("Read [%d]\n", size);

        // elf_exec(buffer, size, user_stack, user_stack_size, ret);
    }
}
