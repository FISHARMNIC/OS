#include <elf.h>
#include <graphics.h>
#include <os_setjmp.h>
#include <fat.h>

#define user_stack_size 4096

uint8_t user_stack[user_stack_size] __attribute__((aligned(user_stack_size)));

// #include "../../userspace/gen/inc/test.h"

void elftest(iret_return_fn_t ret)
{
    (void)ret;
    tty_clear();
    // elf_exec(_Users_nico_Documents_OS_userspace_util____gen_bin_test_elf, _Users_nico_Documents_OS_userspace_util____gen_bin_test_elf_len, user_stack, user_stack_size, ret);
    

    FAT_file_info_t info;
    FAT_read_entry_resp_t resp = fat32_find_file(&info, fat32_get_root(), "TEST", "ELF", true);
    
    if(resp == FILE_FOUND)
    {
        tty_puts("Found file\n");

        uint32_t buffer_size = info.entry.fileSizeBytes;
        uint8_t buffer[buffer_size];

        fat32_load_file(&info, buffer, buffer_size);

        tty_printf("Read [%d]\n", buffer_size);

        elf_exec(buffer, buffer_size, user_stack, user_stack_size, ret);
    }
    else
    {
        tty_puts("[ERROR] File not found\n");
    }
}
