#include <stdint.h>

#include <boot.h>
#include <graphics.h>
#include <keyboard.h>

void kernel_entry(multiboot_info_t* mbi)
{
    postboot_init(mbi);

    tty_puts("Booted!\n");

    terminal();
}