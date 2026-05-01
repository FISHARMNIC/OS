#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <disk.h>
#include <stdbool.h>

#define FAT_BAD_CLUSTER 0x0FFFFFF7

#define FAT_CLUSER_IS_FREE(n) (((n) == 0x10000000) || ((n) == 0xF0000000) || ((n) == 0x00000000))
#define FAT_CLUSTER_IS_VALID(n) ((n) >= 0x00000002 && (n) <= 0x0FFFFFF6)
#define FAT_CLUSTER_IS_EOF(n) ((n) >= 0x0FFFFFF8)

typedef struct
{
    uint32_t rootDirSectors;     // number of sectors occupied by root
    uint32_t total_sectors;      // sectors per fat
    uint32_t fat_size;
    uint32_t firstFatSector;
    uint32_t firstDataSector;    // first cluster 2 sector ON VOLUME. To get global, add BPB_VOLUME_OFFSET
    uint32_t dataSectorCount;    // sector count in data region
    uint32_t clusterCount;       // number of clusters
    uint32_t rootSec;            // root directory
} FAT_info_t;

typedef struct
{
    uint32_t secNum;
    uint32_t entryOffset;
} FAT_cluster_info_t;

typedef struct
{
    char BS_jmpBoot[3];
    char BS_OEMName[8];
    uint16_t BPB_BytsPerSec; // should be: 512
    uint8_t BPB_SecPerClus;  // should be: 1
    uint16_t BPB_RsvdSecCnt; // should be: 32
    uint8_t BPB_NumFATs;     // should be: 2
    uint16_t BPB_RootEntCnt; // should be: 0
    uint16_t BPB_TotSec16;   // should be: 0 (uses TotSec32 instead)
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    char BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    char BS_VolLab[11];
    char BS_FilSysType[8];
} __attribute__((packed)) bpb_raw_t;



#define FAT_ENTRY_ATTR_READ_ONLY 0x01
#define FAT_ENTRY_ATTR_HIDDEN    0x02
#define FAT_ENTRY_ATTR_SYSTEM    0x04
#define FAT_ENTRY_ATTR_VOLUME_ID 0x08
#define FAT_ENTRY_ATTR_DIRECTORY 0x10
#define FAT_ENTRY_ATTR_ARCHIVE   0x20
#define FAT_ENTRY_ATTR_LONG_NAME (FAT_ENTRY_ATTR_READ_ONLY | FAT_ENTRY_ATTR_HIDDEN | FAT_ENTRY_ATTR_SYSTEM | FAT_ENTRY_ATTR_VOLUME_ID)

#define FAT_ENTRY_IS_FREE(name) ((name)[0] == 0xE5 || (name)[0] == 0x00)
#define FAT_NAME_IS_KANJI(name) ((name)[0] == 0x05)
#define FAT_ENTRY_IS_END(name) ((name)[0] == 0x00)

typedef struct __attribute__((packed))
{
    char fileName[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t creationTimeSec;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t clusNumHigh;
    uint16_t lastModifiedTime;
    uint16_t lastModifiedDate;
    uint16_t clusNumLow;
    uint32_t fileSizeBytes;
} FAT_entry_t;

typedef struct
{
    uint8_t order;
    char firstChars[10];
    uint8_t _attribute;
    uint8_t longEntryType;
    uint8_t checkSum;
    char secChars[12];
    uint16_t _reserved;
    char lastChars[4];
} FAT_longFileName_t;

typedef enum
{
    FILE_ERROR,
    FILE_UNUSED,
    FILE_END,
    FILE_FOUND,
} FAT_read_entry_resp_t;

typedef struct __attribute__((packed))
{
    char name[256]; // @todo horrifying. Use malloc. 4 files is already 1k
    uint8_t name_len;
    uint8_t extension_len;
    uint32_t extension_begin;
    bool directory;
} FAT_filename_info_t;

typedef struct
{
    // uint32_t size;
    FAT_filename_info_t name;
    FAT_entry_t entry;
    uint32_t cluster;
} FAT_file_info_t;

uint32_t bpb_init(bpb_raw_t* bpb_info, FAT_info_t* FAT_info);

uint32_t fat32_init();
uint32_t fat32_get_root();

/** 
 * @returns ThisFATSecNum
 */
uint32_t fat32_get_sector_num(uint32_t cluster);

/**
 * @returns ThisFATEntOffset
 */
uint32_t fat32_get_entry_offset(uint32_t cluster);

/**
 * @brief loads @p dest_sector_buff with the data from the cluster number @p cluster
 * @returns the next cluster
 */
uint32_t fat32_next_cluster(ata_rw_data_t dest_sector_buff, uint32_t cluster);
uint32_t fat32_cluster_set_info();

FAT_read_entry_resp_t fat32_read_entry_info(FAT_filename_info_t* resp, FAT_entry_t* info);
void fat32_load_sector(ata_rw_data_t dest_buffer, uint32_t sector);

uint32_t fat32_first_sector_of_cluster(uint32_t cluster);
uint8_t fat32_get_sec_per_clus();

uint32_t fat32_entry_cluster(FAT_entry_t* entry);

/**
 * @warning FILENAME MUST BE UPPERCASE
 * @param info Loads info into this
 */
FAT_read_entry_resp_t fat32_find_file(FAT_file_info_t* info, uint32_t start_cluster, char* name, char* extension, bool recursive);

uint32_t fat32_load_file(FAT_file_info_t* info, uint8_t* buffer, uint32_t max_size);

uint8_t fat_entry_is_dot(FAT_entry_t *entry);

#endif
