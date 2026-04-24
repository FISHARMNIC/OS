#include <disk.h>
#include <ports.h>
#include <graphics.h>

uint8_t sector_count = 1;

uint32_t ata_send_identify(ATA_identify_response_t *resp)
{
    /*
    From OSdev:

    1. select a target drive by sending 0xA0 to the "drive select" IO port => 0x1F6.
    2. Set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5).
    3. Send the IDENTIFY command (0xEC) to the Command IO port (0x1F7).
    4. Read the Status port (0x1F7) again. If the value read is 0, the drive does not exist.
    5. Poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears.
    6. Check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. If so, the
    drive is not ATA, and you should stop polling.
    7. Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
    8. At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0).
    Read 256 16-bit values, and store them.
    */
    // ATA_WAIT_RDY();

    outb(ATA_PORT_DRIVE_SELECT, ATA_DRIVE_MASTER);

    outb(ATA_PORT_SECTOR_COUNT, 0);
    outb(ATA_PORT_LBA_LOW, 0);
    outb(ATA_PORT_LBA_MID, 0);
    outb(ATA_PORT_LBA_HIGH, 0);

    tty_puts("\tSending IDENTIFY\n");

    outb(ATA_PORT_COMMAND, ATA_COMMAND_IDENTIFY);

    tty_puts("\tWaiting BSY\n");

    if (inb(ATA_PORT_STATUS) == 0)
    {
        tty_puts("[ERROR] Drive does not exist");
        return 1;
    }

    ATA_WAIT_BSY();

    tty_puts("\tWaiting DRQ\n");

    ATA_WAIT_DRQ();

    tty_puts("\tReading Drive Format Status\n");

    if (inb(ATA_PORT_LBA_MID) || inb(ATA_PORT_LBA_HIGH))
    {
        tty_puts("\t[ERROR] Drive is not ATA");
        return 1;
    }

    ATA_WAIT_DRQ();

    if (inb(ATA_PORT_STATUS) & ATA_STATUS_ERR)
    {
        tty_puts("\t[ERROR] ATA error\n");
        return 1;
    }

    tty_puts("\tIDENTIFY succesful, reading info\n");

    uint32_t i = 0;
    if (resp == 0)
    {
        while (i < 256)
        {
            inw(ATA_PORT_DATA);
            i++;
        }
    }
    else
    {
        while (i < 256)
        {
            *resp[i++] = inw(ATA_PORT_DATA);
        }
    }

    return 0;
}

void ata_read_sector(ata_rw_data_t arr, uint32_t LBA)
{
    /*
    osDev:

    Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: outb(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
    Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): outb(0x1F1, 0x00)
    Send the sectorcount to port 0x1F2: outb(0x1F2, (unsigned char) count)
    Send the low 8 bits of the LBA to port 0x1F3: outb(0x1F3, (unsigned char) LBA))
    Send the next 8 bits of the LBA to port 0x1F4: outb(0x1F4, (unsigned char)(LBA >> 8))
    Send the next 8 bits of the LBA to port 0x1F5: outb(0x1F5, (unsigned char)(LBA >> 16))
    Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
    Wait for an IRQ or poll.
    Transfer 256 16-bit values, a uint16_t at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
    Then loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
    */
    outb(ATA_PORT_DRIVE_SELECT, 0xE0 | ((LBA >> 24) & 0xF));

    ATA_WAIT_RDY();

    outb(ATA_PORT_DRIVE_SELECT, 0xE0 | ((LBA >> 24) & 0xF)); // outb(0x1f6, 0xE0);

    outb(ATA_PORT_SECTOR_COUNT, sector_count);

    outb(ATA_PORT_LBA_LOW, (uint8_t)(LBA));
    outb(ATA_PORT_LBA_MID, (uint8_t)(LBA >> 8));
    outb(ATA_PORT_LBA_HIGH, (uint8_t)(LBA >> 16));

    outb(ATA_PORT_COMMAND, ATA_COMMAND_READ); // Read

    ATA_WAIT_BSY();
    ATA_WAIT_DRQ();

    uint32_t i = 0;
    while (i < 256)
    {
        arr[i] = inw(ATA_PORT_DATA);
        i++;
    }
}

void ata_write_sector(ata_rw_data_t src, uint32_t LBA)
{
    /*
    To write sectors in 28 bit PIO mode, send command "WRITE SECTORS" (0x30) to the Command port.
    Do not use REP OUTSW to transfer data. There must be a tiny delay between each OUTSW output uint16_t.
    A jmp $+2 size of delay.
    Make sure to do a Cache Flush (ATA command 0xE7) after each write command completes.
    */
    ATA_WAIT_BSY();

    outb(ATA_PORT_DRIVE_SELECT, 0xE0 | ((LBA >> 24) & 0xF));

    outb(ATA_PORT_SECTOR_COUNT, sector_count);
    outb(ATA_PORT_LBA_LOW, (uint8_t)(LBA));
    outb(ATA_PORT_LBA_MID, (uint8_t)(LBA >> 8));
    outb(ATA_PORT_LBA_HIGH, (uint8_t)(LBA >> 16));

    outb(ATA_PORT_COMMAND, ATA_COMMAND_WRITE); // Send the write command
    // outb(0x1F7, 0x30);

    ATA_WAIT_BSY();
    ATA_WAIT_DRQ();

    uint32_t i = 0;
    while (i < 256)
    {
        outw(ATA_PORT_DATA, src[i]);
        NOP_DELAY(2);
        i++;
    }

    ATA_WAIT_BSY();
    outb(ATA_PORT_COMMAND, ATA_COMMAND_CACHE_FLUSH);
    ATA_WAIT_BSY();
}