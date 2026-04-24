#include <boot.h>
#include <fat.h>
#include <graphics.h>

static uint8_t fat_entry_is_dot(FAT_entry_t* entry)
{
    char n0 = entry->fileName[0];
    char n1 = entry->fileName[1];
    char n2 = entry->fileName[2];

    if (n0 != '.')
    {
        return 0;
    }
    else if(n1 == ' ' || (n1 == '.' && n2 == ' '))
    {
        return 1;
    }
    else{
        return 0;
    }
}

static void fat32_walk_dir(uint32_t start_cluster, uint32_t depth)
{
    ata_rw_data_t fat_buffer;
    ata_rw_data_t sector_buffer;
    uint8_t sec_per_clus = fat32_get_sec_per_clus();
    uint32_t cluster = start_cluster;

    if (!FAT_CLUSTER_IS_VALID(cluster))
    {
        tty_printf("Invalid directory start cluster: %d\n", cluster);
        return;
    }

    while (1)
    {
        tty_printf("Depth %d, reading cluster %d\n", depth, cluster);

        uint32_t first_sector = fat32_first_sector_of_cluster(cluster);

        for (uint32_t sec = 0; sec < sec_per_clus; sec++)
        {
            ata_read_sector(sector_buffer, SECTOR_TO_LBA(first_sector + sec));

            FAT_entry_t* entries = (FAT_entry_t*)sector_buffer;

            for (int i = 0; i < 16; i++)
            {
                FAT_entry_t* entry = &entries[i];
                FAT_file_info_t info;
                FAT_read_entry_resp_t resp = fat32_read_entry_info(&info, entry);

                if (resp == FILE_END)
                {
                    return;
                }
                if (resp != FILE_FOUND)
                {
                    continue;
                }
                else
                {
                    tty_printf("Found entry: %s\n", info.name);
                }

                if ((entry->attributes & FAT_ENTRY_ATTR_DIRECTORY) == 0)
                {
                    continue;
                }
                if (entry->attributes == FAT_ENTRY_ATTR_LONG_NAME || fat_entry_is_dot(entry))
                {
                    continue;
                }

                if (depth >= 16)
                {
                    tty_puts("Max recursion depth reached, skipping nested directory\n");
                    continue;
                }

                uint32_t child_cluster = fat32_entry_cluster(entry);
                
                if (!FAT_CLUSTER_IS_VALID(child_cluster))
                {
                    continue;
                }

                fat32_walk_dir(child_cluster, depth + 1);
            }
        }

        uint32_t next = fat32_next_cluster(fat_buffer, cluster);
        if (FAT_CLUSTER_IS_EOF(next))
        {
            return;
        }
        if (!FAT_CLUSTER_IS_VALID(next))
        {
            tty_printf("Invalid next cluster value: %d\n", next);
            return;
        }

        cluster = next;
    }
}

void fattest()
{
    tty_clear();
    fat32_walk_dir(fat32_get_root(), 0);
}
