#include <stdint.h>

#include <boot.h>
#include <graphics.h>

void kernel_entry(multiboot_info_t* mbi)
{
    postboot_init(mbi);

    graphics_draw_string("Booted", 0, 0, &graphics_context_default);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
