#include <fat.h>
#include <graphics.h>
#include <cpu.h>
#include <stdbool.h>
#include <sys/string.h>

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

void __fat32_walk_dir(uint32_t start_cluster, uint32_t depth)
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
            fat32_load_sector(sector_buffer, first_sector + sec);

            FAT_entry_t* entries = (FAT_entry_t*)sector_buffer;

            for (int i = 0; i < 16; i++)
            {
                FAT_entry_t* entry = &entries[i];
                FAT_filename_info_t info;
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
                    tty_printf("Found entry[%s]: %s.%s\n", info.directory ? "dir" : "file", info.name, info.extension_begin);
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

                __fat32_walk_dir(child_cluster, depth + 1);
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

FAT_read_entry_resp_t fat32_find_file(FAT_file_info_t* info, uint32_t start_cluster, char* name, char* extension, bool recursive)
{
    ata_rw_data_t fat_buffer;
    ata_rw_data_t sector_buffer;
    uint8_t sec_per_clus = fat32_get_sec_per_clus();
    uint32_t cluster = start_cluster;

    if (!FAT_CLUSTER_IS_VALID(cluster))
    {
        tty_printf("Invalid directory start cluster: %d\n", cluster);
        return FILE_ERROR;
    }

    while (1)
    {
        uint32_t first_sector = fat32_first_sector_of_cluster(cluster);

        for (uint32_t sec = 0; sec < sec_per_clus; sec++)
        {
            fat32_load_sector(sector_buffer, first_sector + sec);

            FAT_entry_t* entries = (FAT_entry_t*)sector_buffer;

            uint32_t i = 0;
            while(1)
            {
                // FAT_entry_t* entry = &(info->entry);

                // *entry = entries[i++];
                FAT_entry_t* entry = &entries[i++];

                // memcpy(entry, &, sizeof(FAT_entry_t));

                FAT_filename_info_t curr_info;
                FAT_read_entry_resp_t resp = fat32_read_entry_info(&curr_info, entry);

                if (resp == FILE_END)
                {
                    return FILE_END;
                }
                if (resp != FILE_FOUND)
                {
                    continue;
                }
                else
                {
                    // tty_printf("Comparing %s %s %d\n", name, curr_info.name);
                    if(strcmp(name, curr_info.name) == 0)
                    {
                        // tty_printf("MATCH NAME\n");
                        if(extension == NULLPTR || (strcmp(extension, curr_info.name + curr_info.extension_begin) == 0))
                        {
                            // tty_printf("---FOUND '%s'---\n", name);
                            // info->cluster = cluster;
                            info->cluster = fat32_entry_cluster(entry);
                            info->name = curr_info;
                            info->entry = *entry;
                            // info->

                            return FILE_FOUND;
                        }
                    }
                    
                }
                
                if ((entry->attributes & FAT_ENTRY_ATTR_DIRECTORY) == 0)
                {
                    continue;
                }
                if (fat_entry_is_dot(entry))
                {
                    continue;
                }

                uint32_t child_cluster = fat32_entry_cluster(entry);
                
                if (!FAT_CLUSTER_IS_VALID(child_cluster))
                {
                    continue;
                }

                if(recursive)
                {
                    FAT_read_entry_resp_t ret = fat32_find_file(info, child_cluster, name, extension, recursive);
                    if(ret == FILE_FOUND)
                    {
                        return FILE_FOUND;
                    }
                }
            }
        }

        uint32_t next = fat32_next_cluster(fat_buffer, cluster);
        
        if (FAT_CLUSTER_IS_EOF(next))
        {
            // return FILE_END;
            break;
        }
        if (!FAT_CLUSTER_IS_VALID(next))
        {
            // tty_printf("Invalid next cluster value: %d\n", next);
            return FILE_ERROR;
        }

        cluster = next;
    }

    return 1;
}

uint32_t fat32_load_file(FAT_file_info_t* info, uint8_t* buffer, uint32_t max_size)
{
    uint8_t sec_per_clus = fat32_get_sec_per_clus();
    
    uint32_t cluster = info->cluster;
    uint32_t file_size = info->entry.fileSizeBytes;
    uint32_t remaining = (file_size < max_size) ? file_size : max_size;
    
    ata_rw_data_t sector_buf;
    
    uint32_t cluster_count = 0;
    
    while (FAT_CLUSTER_IS_VALID(cluster) && remaining > 0)
    {
        cluster_count++;
        uint32_t first_sector = fat32_first_sector_of_cluster(cluster);

        for (uint32_t s = 0; s < sec_per_clus && remaining > 0; s++)
        {
            uint32_t current_sector = first_sector + s;
  
            fat32_load_sector(sector_buf, current_sector);
            
            uint32_t to_copy = 512;
            if (to_copy > remaining)
                to_copy = remaining;
            
            memcpy(buffer, sector_buf, to_copy);
            buffer += to_copy;
            remaining -= to_copy;
        }
        
        uint32_t next_cluster = fat32_next_cluster(sector_buf, cluster);

        cluster = next_cluster;
        
        if (!FAT_CLUSTER_IS_VALID(cluster) && remaining > 0)
        {
            // tty_printf("[ERROR] Invalid cluster %d encountered with %d bytes remaining!\n", cluster, remaining);
            return 0;
        }
    }
    
    uint32_t bytes_read = (file_size < max_size ? file_size : max_size) - remaining;

    return bytes_read;
}