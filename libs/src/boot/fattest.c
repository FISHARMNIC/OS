#include <boot.h>
#include <fat.h>
#include <graphics.h>

void fattest()
{
    tty_clear();
    // fat32_walk_dir(fat32_get_root(), 0);
    FAT_entry_t entry;
    FAT_read_entry_resp_t resp = fat32_find_file(&entry, fat32_get_root(), "TEST", "ELF", true);
    
    if(resp == FILE_FOUND)
    {
        tty_puts("Found file\n");
    }
    else
    {
        tty_puts("[ERROR] File not found\n");
    }
}
