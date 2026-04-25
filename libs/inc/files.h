#ifndef FILES_H
#define FILES_H

#include <fat.h>

typedef FAT_file_info_t fd_t;

/**
 * @param fd loads info into this
 */
uint32_t file_find(fd_t *fd, char *name);
uint32_t file_size(fd_t *info);
uint32_t file_read(fd_t *info, uint8_t *buffer, uint32_t size);

#endif