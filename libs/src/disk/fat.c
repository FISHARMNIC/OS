#include <disk.h>
#include <fat.h>
#include <graphics.h>
#include <sys/string.h>

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
    uint32_t i = 0;
    while (resp->name[i] != ' ' && i < 255)
    {
        i++;
    }

    resp->name_len = i;
    resp->name[i] = 0;
    
    i++;

    while (resp->name[i] == ' ' && i < 255)
    {
        i++;
    }
    resp->extension_begin = i;

    uint32_t len = 0;
    while (resp->name[i] != ' ' && i < 255)
    {
        i++;
        len++;
    }
    resp->extension_len = len;
    resp->name[i] = 0;
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