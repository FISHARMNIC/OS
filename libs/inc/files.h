#ifndef FILES_H
#define FILES_H

#include <fat.h>

typedef FAT_file_info_t fd_t;

typedef enum {
    LS_OK,
    LS_ERROR_CLUSTER,
    LS_ERROR_MAX_SIZE,

} files_ls_resp_t;
/**
 * @param fd loads info into this
 */
uint32_t file_find(fd_t *fd, char *name);
uint32_t file_size(fd_t *info);
uint32_t file_read(fd_t *info, uint8_t *buffer, uint32_t size);

int32_t files_ls(fd_t infos[], uint32_t max_size, uint32_t start_cluster);

#define PWD_SIZE 100
extern char pwd_temp[PWD_SIZE];
extern char pwd[PWD_SIZE];

#endif