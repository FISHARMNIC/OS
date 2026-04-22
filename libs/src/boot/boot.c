#include <graphics.h>
#include <boot.h>

void postboot_init(multiboot_info_t* mbi)
{
    // Setup default framebuffer
    graphics_init_fb(&graphics_fb_default, mbi);
    graphics_fb_active = &graphics_fb_default;

    // Setup default context
    graphics_init_context(&graphics_context_default, &graphics_fb_default, _binary_FONT_F16_start, COLOR_WHITE, COLOR_BLACK);
}