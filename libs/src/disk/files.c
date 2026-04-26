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

    if (name == NULLPTR)
    {
        fat32_find_file(fd, 0, NULLPTR, NULLPTR, false);
        return 0;
    }

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
        // tty_printf("\tLooking for: %s\n", ptr);

        FAT_read_entry_resp_t resp = fat32_find_file(fd, cluster, ptr, NULLPTR, false);
        if (resp != FILE_FOUND)
        {
            // tty_printf("\tUnable to find it\n");
            return 1;
        }

        cluster = fd->cluster;
        // tty_printf("\tIn Cluster: %d\n", cluster);
        ptr = nextptr;
    }

    FAT_read_entry_resp_t resp;

    if (*ptr == '.') // ../ and ./
    {
        if (ptr[1] == '.')
        {
            resp = fat32_find_file(fd, cluster, "..", NULLPTR, false);
        }
        else
        {
            resp = fat32_find_file(fd, cluster, ".", NULLPTR, false);
        }
    }
    else
    {

        char *nextptr = find_next(ptr, '.');

        // tty_printf("Now onto file '%s'.'%s'\n", ptr, nextptr == NULLPTR ? "NONE" : nextptr);

        resp = fat32_find_file(fd, cluster, ptr, nextptr, false);
    }

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

// @todo should return/write a list of fd_t
int32_t files_ls(fd_t infos[], uint32_t max_size, uint32_t start_cluster)
{
    ata_rw_data_t fat_buffer;
    ata_rw_data_t sector_buffer;
    uint8_t sec_per_clus = fat32_get_sec_per_clus();
    uint32_t cluster = start_cluster;

    if (start_cluster == 0) // @todo maybe not the best way to approach ls from root? If something else calls expecting 0 to fail this breaks
    {
        cluster = fat32_get_root();
    }

    if (!FAT_CLUSTER_IS_VALID(cluster))
    {
        tty_printf("Invalid directory start cluster: %d\n", cluster);
        return -LS_ERROR_CLUSTER;
    }

    uint32_t n = 0;

    while (1)
    {
        uint32_t first_sector = fat32_first_sector_of_cluster(cluster);

        for (uint32_t sec = 0; sec < sec_per_clus; sec++)
        {
            fat32_load_sector(sector_buffer, first_sector + sec);

            FAT_entry_t *entries = (FAT_entry_t *)sector_buffer;

            for (uint32_t i = 0; i < 16; i++)
            {
                FAT_entry_t *entry = &entries[i];

                if (FAT_ENTRY_IS_END(entry->fileName))
                {
                    return n;
                }

                if (FAT_ENTRY_IS_FREE(entry->fileName))
                {
                    continue;
                }

                if (entry->attributes == FAT_ENTRY_ATTR_LONG_NAME)
                {
                    continue;
                }

                FAT_filename_info_t info;
                FAT_read_entry_resp_t resp = fat32_read_entry_info(&info, entry);

                if (resp != FILE_FOUND)
                {
                    continue;
                }

                if (n >= max_size)
                {
                    return -LS_ERROR_MAX_SIZE;
                }

                infos[n].name = info;
                infos[n].cluster = fat32_entry_cluster(entry);
                infos[n].entry = *entry;

                n++;

                // if(info.directory)
                // {
                //     tty_printf("%s/\n", info.name);
                // }
                // else
                // {
                //     tty_printf("%s.%s\n", info.name, info.name + info.extension_begin);
                // }
            }
        }

        uint32_t next = fat32_next_cluster(fat_buffer, cluster);

        if (FAT_CLUSTER_IS_EOF(next) || !FAT_CLUSTER_IS_VALID(next))
            return n;

        cluster = next;
    }

    return n;
}