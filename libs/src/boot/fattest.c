#include <boot.h>
#include <fat.h>
#include <graphics.h>
#include <files.h>

void fattest()
{
    tty_clear();

    files_ls(fat32_get_root());
   
    // fd_t fd;

    // uint32_t err = file_find(&fd, "DIR1/DIR1A/HELLO.TXT");
    // if(err)
    // {
    //     tty_printf("[ERROR] Could Not find\n");
    // }
    // else
    // {
    //     uint32_t size = file_size(&fd);
    //     uint8_t buffer[size];

    //     file_read(&fd, buffer, size);

    //     tty_printf("Read: '%s'\n", buffer);
    // }
}
