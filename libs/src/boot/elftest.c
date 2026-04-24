#include <elf.h>
#include <graphics.h>

#define user_stack_size 4096

uint8_t user_stack[user_stack_size] __attribute__((aligned(user_stack_size)));

#include "../../userspace/gen/inc/test.h"

void elftest()
{
    tty_clear();
    elf_exec(_Users_nico_Documents_OS_userspace_util____gen_bin_test_elf, _Users_nico_Documents_OS_userspace_util____gen_bin_test_elf_len, user_stack, user_stack_size);
}