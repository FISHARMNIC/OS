#include <syscalls.h>

char* s = "Hello World";

int main(void)
{
    SYSCALL(SYSCALL_PUTS, s);

    return 0;
}