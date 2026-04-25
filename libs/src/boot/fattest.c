#include <boot.h>
#include <fat.h>
#include <graphics.h>

void fattest()
{
    tty_clear();
    // fat32_walk_dir(fat32_get_root(), 0);
    FAT_file_info_t info;
    FAT_read_entry_resp_t resp = fat32_find_file(&info, fat32_get_root(), "TEST", "TXT", true);
    
    if(resp == FILE_FOUND)
    {
        tty_puts("Found file\n");
        uint8_t buffer[info.entry.fileSizeBytes];
        fat32_load_file(&info, buffer, info.entry.fileSizeBytes);
        tty_printf("Read [%d]: %s\n", info.entry.fileSizeBytes, buffer);
    }
    else
    {
        tty_puts("[ERROR] File not found\n");
    }
}
