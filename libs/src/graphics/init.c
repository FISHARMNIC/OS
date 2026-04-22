#include <graphics.h>

framebuffer_t graphics_fb_default;
framebuffer_t* graphics_fb_active;

graphics_context_t graphics_context_default;

void graphics_init_fb(framebuffer_t* fb, multiboot_info_t* mbi)
{
    fb->addr = (uint32_t)mbi->framebuffer_addr;
    fb->pitch = mbi->framebuffer_pitch;
    fb->width = mbi->framebuffer_width;
    fb->height = mbi->framebuffer_height;
    fb->bpp = mbi->framebuffer_bpp;
}

void graphics_init_context(graphics_context_t* context, framebuffer_t* fb, const uint8_t* font, uint32_t color_fg, uint32_t color_bg)
{
    context->fb = fb;
    context->color_fg = color_fg;
    context->color_bg = color_bg;
    context->font = font;
}