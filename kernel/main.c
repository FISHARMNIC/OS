#include <stdint.h>

#include <boot.h>
#include <graphics.h>
#include <keyboard.h>

void kernel_entry(multiboot_info_t* mbi)
{
    uint32_t pb_failiure = postboot_init(mbi);
    if(pb_failiure)
    {
        tty_puts("-- POST BOOT FAILED --");
        asm volatile("cli; hlt;");
    }

    tty_puts("Booted!\n");

    // fattest();
    // elftest(terminal);
    
    terminal();

    asm volatile("cli; hlt;");
}