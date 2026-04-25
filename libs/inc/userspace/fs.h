#ifndef FS_H
#define FS_H

#include <syscalls.h>
#include <stdint.h>
#include <files.h>

// @todo make actual functions

static inline uint32_t fsize(fd_t* fd) {
    return fd->entry.fileSizeBytes;
}

static inline uint32_t ffind(fd_t* fd, char* name) {

    uint32_t err;
    SYSCALL_3PARAM(SYSCALL_FILE_FIND, fd, name, &err);
    
    return err;

}

static inline uint32_t fread(fd_t* fd, char* buffer, uint32_t size)
{
    uint32_t bytes_written;
    SYSCALL_4PARAM(SYSCALL_FILE_READ, fd, buffer, size, &bytes_written);

    return bytes_written;
}

static inline uint32_t fls(fd_t infos[], fd_t* start, uint32_t size)
{
    //  fd_t **fd_arr = (fd_t **)registers.SYSCALL_PARAM_1;
    // uint32_t size = (uint32_t)registers.SYSCALL_PARAM_2;
    // fd_t *fd = (fd_t *)registers.SYSCALL_PARAM_1;
    // uint32_t *resp = (uint32_t *)registers.SYSCALL_PARAM_4;

    // *resp = files_ls(fd_arr, size, fd->cluster);

    uint32_t count;
    SYSCALL_4PARAM(SYSCALL_FILE_LS, infos, size, start, &count);

    return count;
}

#endif
