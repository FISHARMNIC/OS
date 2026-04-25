#include <files.h>
#include <sys/string.h>
#include <cpu.h>
#include <graphics.h>

// @todo replace with use of strtok_r
static char *find_next(char *ptr, char find) // replaces slash will null and returns next section
{
    char *nextptr = NULLPTR;

    nextptr = strchr(ptr, find);
    if (nextptr == NULLPTR)
    {
        // tty_printf("\tNULL\n");
        return NULLPTR;
    }

    *nextptr = 0;
    ptr = nextptr + 1;

    return ptr;
}

uint32_t file_find(fd_t *fd, char *name)
{
    // tty_printf("SCANNING %s\n", name);

    uint32_t len = strlen(name) + 1;

    char buffer[len];
    char *ptr = buffer;

    memcpy(ptr, name, len + 1);

    uint32_t cluster = fat32_get_root();

    while (1)
    {
        char *nextptr = find_next(ptr, '/');
        if (nextptr == NULLPTR || (uint32_t)(ptr - buffer) > len)
        {
            // tty_printf("END %d %d\n", (uint32_t)(ptr - buffer), len);
            break; // on to file now
        }
        // tty_printf("Looking for: %s\n", ptr);

        FAT_read_entry_resp_t resp = fat32_find_file(fd, cluster, ptr, NULLPTR, false);
        if (resp != FILE_FOUND)
        {
            // tty_printf("\tUnable to find it\n");
            return 1;
        }

        cluster = fd->cluster;
        ptr = nextptr;
    }

    char *nextptr = find_next(ptr, '.');

    // tty_printf("Now onto file '%s'.'%s'\n", ptr, nextptr == NULLPTR ? "NONE" : nextptr);

    FAT_read_entry_resp_t resp = fat32_find_file(fd, cluster, ptr, nextptr, false);

    if (resp == FILE_FOUND)
    {
        // tty_printf("\tFound it!\n");
        return 0;
    }
    else
    {
        // tty_printf("\tUnable to find it\n");
        return 1;
    }

    /*
    split by slash into: dir1 dir2 dir2 ... file
    then do:
        cluster1 = fat32_find_file(root, dir1)
        cluster2 = fat32_find_file(cluster2, dir2)
        ...
        clusterM = fat32_find_file(clusterN, dirN)

    */
}

uint32_t file_size(fd_t *info)
{
    return info->entry.fileSizeBytes;
}

uint32_t file_read(fd_t *info, uint8_t *buffer, uint32_t size)
{
    return fat32_load_file(info, buffer, size);
}