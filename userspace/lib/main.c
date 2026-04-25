#include <syscalls.h>

extern int main();

void _start()
{
    int ret = main();
    SYSCALL(SYSCALL_EXIT, ret);
}