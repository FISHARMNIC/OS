#ifndef USER_SYS_H
#define USER_SYS_H

#include <syscalls.h>
#include <stdint.h>
#include <files.h>

static inline uint32_t exec(fd_t* fd, uint32_t argc, char** argv)
{
    uint32_t resp;
    
    SYSCALL_4PARAM(SYSCALL_EXEC, fd, argc, argv, &resp);

    return resp;
}


#endif