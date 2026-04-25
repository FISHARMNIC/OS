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
    SYSCALL_3PARAM(SYSCALL_FILE_FIND, &fd, name, &err);
    
    return err;

}

static inline uint32_t fread(fd_t* fd, char* buffer, uint32_t size)
{
    uint32_t bytes_written;
    SYSCALL_4PARAM(SYSCALL_FILE_FIND, fd, buffer, size, &bytes_written);

    return bytes_written;
}


#endif
