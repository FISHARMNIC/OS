#include <syscalls.h>

extern int main(int argc, char *argv[]);

void _start(int argc, char *argv[])
{
    int ret = main(argc, argv);
    SYSCALL(SYSCALL_EXIT, ret);
}