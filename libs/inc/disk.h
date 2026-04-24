#ifndef DISK_H
#define DISK_H

#include <stdint.h>

// mostly from fatgen103

#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_RDY 0x40
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_DF  0x20
#define ATA_STATUS_ERR 0x01

#define ATA_COMMAND_IDENTIFY  0xEC
#define ATA_COMMAND_READ      0x20
#define ATA_COMMAND_WRITE     0x30
#define ATA_COMMAND_CACHE_FLUSH 0xE7

#define ATA_PORT_DATA         0x1F0
#define ATA_PORT_SECTOR_COUNT 0x1F2
#define ATA_PORT_LBA_LOW      0x1F3
#define ATA_PORT_LBA_MID      0x1F4
#define ATA_PORT_LBA_HIGH     0x1F5
#define ATA_PORT_DRIVE_SELECT 0x1F6
#define ATA_PORT_COMMAND      0x1F7
#define ATA_PORT_STATUS       0x1F7

#define ATA_DRIVE_MASTER      0xA0

#define ATA_WAIT_RDY() while (!(inb(ATA_PORT_COMMAND) & ATA_STATUS_RDY))
#define ATA_WAIT_BSY() while (inb(ATA_PORT_COMMAND) & ATA_STATUS_BSY)
#define ATA_WAIT_DRQ() while (!(inb(ATA_PORT_COMMAND) & ATA_STATUS_DRQ))

#define BPB_VOLUME_OFFSET 2048

#define NOP_DELAY(s)                   \
    do {                               \
        for (uint32_t i = 0; i < s; i++)    \
            asm("nop");                \
    } while (0)


#define SECTOR_TO_LBA(sector) (BPB_VOLUME_OFFSET + (sector))

// offset table https://wiki.osdev.org/FAT#FAT_32 -> imp. details, BPB
// Note, the bpb is located at the first sector of the VOLUME, not the disk. -> the first sector formatted with FAT


typedef uint16_t ata_rw_data_t[256];
typedef ata_rw_data_t ATA_identify_response_t;

uint32_t ata_send_identify(ATA_identify_response_t* resp);
void ata_read_sector(ata_rw_data_t arr, uint32_t LBA);
void ata_write_sector(ata_rw_data_t src, uint32_t LBA);

#endif