#include <disk.h>
#include <fat.h>
#include <graphics.h>
#include <sys/string.h>
#include <cpu.h>
#include <stdbool.h>

static FAT_info_t fat_info;
static bpb_raw_t bpb_info;

// static ata_rw_data_t global_buffer;

uint32_t bpb_init(bpb_raw_t *bpb_info, FAT_info_t *FAT_info)
{
    static ata_rw_data_t bpb_buffer;

    // Volume sector 0, located at 1MB (in fat_32)
    tty_puts("\tReading BPB\n");

    ata_read_sector(bpb_buffer, BPB_VOLUME_OFFSET);

    tty_puts("\tSetting FAT info\n");

    memcpy(bpb_info, bpb_buffer, sizeof(bpb_raw_t));

    uint32_t size = bpb_info->BPB_FATSz32;

    FAT_info->total_sectors = bpb_info->BPB_TotSec32;
    FAT_info->fat_size = size;

    FAT_info->rootDirSectors = ((bpb_info->BPB_RootEntCnt * 32) + (bpb_info->BPB_BytsPerSec - 1)) / bpb_info->BPB_BytsPerSec;
    FAT_info->firstDataSector = bpb_info->BPB_RsvdSecCnt + (bpb_info->BPB_NumFATs * size) + FAT_info->rootDirSectors;

    FAT_info->dataSectorCount = FAT_info->total_sectors - (bpb_info->BPB_RsvdSecCnt + (bpb_info->BPB_NumFATs * size) + FAT_info->rootDirSectors);
    FAT_info->clusterCount = FAT_info->dataSectorCount / bpb_info->BPB_SecPerClus;

    if (FAT_info->clusterCount < 65525)
    {
        tty_puts("[ERROR] FS is not Fat32");
        return 1;
    }

    FAT_info->firstFatSector = bpb_info->BPB_RsvdSecCnt;
    FAT_info->rootSec = FAT_info->firstDataSector + (bpb_info->BPB_RootClus - 2) * bpb_info->BPB_SecPerClus;

    return 0;
}

uint32_t fat32_init()
{
    uint32_t err = bpb_init(&bpb_info, &fat_info);

    if (err)
    {
        return err;
    }

    return 0;
}

uint32_t fat32_first_sector_of_cluster(uint32_t cluster)
{
    return (cluster - 2) * bpb_info.BPB_SecPerClus + fat_info.firstDataSector;
}

uint8_t fat32_get_sec_per_clus()
{
    return bpb_info.BPB_SecPerClus;
}

uint32_t fat32_get_sector_num(uint32_t cluster) // ThisFATSecNum
{
    uint32_t offset = cluster * 4;
    return bpb_info.BPB_RsvdSecCnt + (offset / bpb_info.BPB_BytsPerSec);
}

uint32_t fat32_get_entry_offset(uint32_t cluster) // ThisFATEntOffset
{
    uint32_t offset = cluster * 4;
    return offset % bpb_info.BPB_BytsPerSec;
}

uint32_t fat32_next_cluster(ata_rw_data_t dest_sector_buff, uint32_t cluster)
{
    /*
    You now read sector number ThisFATSecNum (remember this is a sector number relative to sector 0
    of the FAT volume). Assume this is read into an 8-bit byte array named SecBuff. Also assume that the
    type WORD is a 16-bit unsigned and that the type DWORD is a 32-bit unsigned.
    */

    uint32_t sector_num = fat32_get_sector_num(cluster);
    uint32_t entry_offset = fat32_get_entry_offset(cluster);

    ata_read_sector(dest_sector_buff, SECTOR_TO_LBA(sector_num));

    uint8_t *as_bytes = (uint8_t *)dest_sector_buff;

    // FAT32ClusEntryVal = (*((DWORD *) &SecBuff[ThisFATEntOffset])) & 0x0FFFFFFF;
    uint32_t cluster_entry_val = *((uint32_t *)(&as_bytes[entry_offset])) & 0x0FFFFFFF;

    return cluster_entry_val;
}

uint32_t fat32_cluster_set_info() // @todo
{
    /*
    To set the contents of this same cluster you do the following:
        *((WORD *) &SecBuff[ThisFATEntOffset]) = FAT16ClusEntryVal;
        FAT32ClusEntryVal = FAT32ClusEntryVal & 0x0FFFFFFF;
        *((DWORD *) &SecBuff[ThisFATEntOffset]) = (*((DWORD *) &SecBuff[ThisFATEntOffset])) & 0xF0000000;
        *((DWORD *) &SecBuff[ThisFATEntOffset]) = (*((DWORD *) &SecBuff[ThisFATEntOffset])) | FAT32ClusEntryVal;
    */

    return 0;
}

void fat32_load_sector(ata_rw_data_t dest_buffer, uint32_t sector)
{
    ata_read_sector(dest_buffer, SECTOR_TO_LBA(sector));
}

static void fat32_parse_name(FAT_filename_info_t *resp)
{
    // Handle . and .. directory entries
    if (resp->name[0] == '.')
    {
        resp->name_len = (resp->name[1] == '.') ? 2 : 1;
        resp->name[resp->name_len] = '\0';
        resp->extension_begin = resp->name_len + 1;
        resp->name[resp->extension_begin] = '\0';
        resp->extension_len = 0;
        return;
    }

    uint32_t i = 0;

    if (/* long filename — contains a dot */ strchr(resp->name, '.') != 0)
    {
        // Long filename format: "HELLO.TXT"
        while (i < 255 && resp->name[i] != '.' && resp->name[i] != '\0')
            i++;

        resp->name_len = i;
        resp->name[i] = '\0';   // null terminate name
        i++;                     // skip dot

        // Copy extension to just after the null terminator
        resp->extension_begin = i;
        uint32_t len = 0;
        while (i < 255 && resp->name[i] != '\0' && resp->name[i] != ' ')
        {
            i++;
            len++;
        }
        resp->name[i] = '\0';
        resp->extension_len = len;
    }
    else
    {
        // Short 8.3 format: "HELLO   TXT"
        while (i < 255 && resp->name[i] != ' ' && resp->name[i] != '\0')
            i++;

        resp->name_len = i;
        resp->name[i] = '\0';   // null terminate name

        // Skip padding spaces
        i++;
        while (i < 255 && resp->name[i] == ' ')
            i++;

        resp->extension_begin = i;
        uint32_t len = 0;
        while (i < 255 && resp->name[i] != ' ' && resp->name[i] != '\0')
        {
            i++;
            len++;
        }
        resp->name[i] = '\0';
        resp->extension_len = len;
    }
}

FAT_read_entry_resp_t fat32_read_entry_info(FAT_filename_info_t *resp, FAT_entry_t *info)
{
    if (FAT_ENTRY_IS_FREE(info->fileName))
    {
        if (FAT_ENTRY_IS_END(info->fileName))
        {
            // tty_puts("\t[FAT] end\n");
            return FILE_END;
        }
        else
        {
            // tty_puts("\t[FAT] unused\n");
            return FILE_UNUSED;
        }
    }

    resp->directory = (info->attributes & FAT_ENTRY_ATTR_DIRECTORY) == FAT_ENTRY_ATTR_DIRECTORY;

    if (info->attributes == FAT_ENTRY_ATTR_LONG_NAME)
    {
        FAT_longFileName_t *filename = (FAT_longFileName_t *)(&(info->fileName));
        // tty_printf("\t[FAT] long file name: %s%s%s\n", filename->firstChars, filename->secChars, filename->lastChars);

        char *dest = resp->name;
        char *src1 = filename->firstChars;
        char *src2 = filename->secChars;
        char *src3 = filename->lastChars;

        while (*src1)
            *(dest++) = *(src1++);
        while (*src2)
            *(dest++) = *(src2++);
        while (*src3)
            *(dest++) = *(src3++);

        *dest = 0;

        fat32_parse_name(resp);

        return FILE_FOUND;
    }
    else
    {
        memset(&(resp->name), 0, sizeof(resp->name));
        memcpy(&(resp->name), info->fileName, 11);

        fat32_parse_name(resp);

        return FILE_FOUND;
    }
}

uint32_t fat32_get_root()
{
    return bpb_info.BPB_RootClus;
}

uint32_t fat32_entry_cluster(FAT_entry_t *entry)
{
    return ((uint32_t)entry->clusNumHigh << 16) | entry->clusNumLow;
}

uint8_t fat_entry_is_dot(FAT_entry_t *entry)
{
    char n0 = entry->fileName[0];
    char n1 = entry->fileName[1];
    char n2 = entry->fileName[2];

    if (n0 != '.')
    {
        return 0;
    }
    else if (n1 == ' ' || (n1 == '.' && n2 == ' '))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

FAT_read_entry_resp_t fat32_find_file(FAT_file_info_t *info, uint32_t start_cluster, char *name, char *extension, bool recursive)
{
    ata_rw_data_t fat_buffer;
    ata_rw_data_t sector_buffer;
    uint8_t sec_per_clus = fat32_get_sec_per_clus();
    uint32_t cluster = start_cluster;

    if(name == NULLPTR)
    {
        info->cluster = start_cluster; // @todo !important load other info too?
        return FILE_FOUND;
        // fat32_load_sector(sector_buffer, fat32_first_sector_of_cluster(fat32_get_root()));
    }

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

            FAT_entry_t *entries = (FAT_entry_t *)sector_buffer;

            uint32_t i = 0;
            while (1)
            {
                // FAT_entry_t* entry = &(info->entry);

                // *entry = entries[i++];
                FAT_entry_t *entry = &entries[i++];

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
                    if (strcmp(name, curr_info.name) == 0)
                    {
                        // tty_printf("MATCH NAME\n");
                        // uint32_t curr_extension_len = strlen(curr_info.name + curr_info.extension_begin); // @todo fix extension_len
                        uint32_t curr_extension_len = curr_info.extension_len;

                        if ((extension == NULLPTR && curr_extension_len == 0) || (strcmp(extension, curr_info.name + curr_info.extension_begin) == 0))
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

                if (recursive)
                {
                    FAT_read_entry_resp_t ret = fat32_find_file(info, child_cluster, name, extension, recursive);
                    if (ret == FILE_FOUND)
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

uint32_t fat32_load_file(FAT_file_info_t *info, uint8_t *buffer, uint32_t max_size)
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