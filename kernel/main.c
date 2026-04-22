#include <stdint.h>

#include <boot.h>
#include <graphics.h>
#include <keyboard.h>

void drawch(uint8_t c)
{
    tty_putch(keyboard_keycode_to_char(c));
}

void kernel_entry(multiboot_info_t* mbi)
{
    postboot_init(mbi);

    tty_puts("Booted!\n");

    keyboard_on_press_fn = drawch;

    // char buff[100];
    for (;;) {
        // keyboard_gets(buff, sizeof(buff));
    }
}
