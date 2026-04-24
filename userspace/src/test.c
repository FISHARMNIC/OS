#include <syscalls.h>

char* s = "Hello World";

void _start(void)
{

    SYSCALL(SYSCALL_PUTS, s);
    while (1) {};
}