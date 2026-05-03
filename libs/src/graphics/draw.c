#include <graphics.h>
#include <boot.h>

// @todo clean this and make formate of userspace one

void graphics_draw_glyph(const uint8_t *glyph, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    const framebuffer_t fb = *ctx->fb;

    const uint8_t bpp = fb.bpp;
    const uint32_t pitch = fb.pitch;
    const uint32_t width = fb.width;
    const uint32_t height = fb.height;
    const uint32_t addr = fb.addr;

    const uint32_t color_fg = ctx->color_fg;
    const uint32_t color_bg = ctx->color_bg;

    const uint32_t cheight = ctx->font.char_height;
    const uint32_t cwidth = ctx->font.char_width;

    for (uint32_t offy = 0; offy < cheight; offy++)
    {
        for (uint32_t offx = 1; offx <= cwidth; offx++)
        {

            const uint32_t sx = x + offx;
            const uint32_t sy = y + offy;

            if ((uint32_t)sx >= width || (uint32_t)sy >= height)
            {
                continue;
            }

            const uint32_t color = (glyph[offy] & MASK(cwidth - offx)) ? color_fg : color_bg;
            const uint8_t *row = (uint8_t *)(addr + (sy * pitch));

            if (bpp == 32)
            {
                ((uint32_t *)row)[sx] = color;
            }
            else if (bpp == 16)
            {
                ((uint16_t *)row)[sx] = (uint16_t)color;
            }
        }
    }
}

void graphics_draw_char(uint8_t character, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    const uint8_t *glyph = ctx->font.ptr + (character * ctx->font.char_height);

    graphics_draw_glyph(glyph, x, y, ctx);
}

void graphics_draw_string(char *string, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    while (*string)
    {
        graphics_draw_char((uint8_t)*string, x, y, ctx);
        x += ctx->font.char_width;
        string++;
    }
}

void graphics_clear_screen(uint32_t color, const framebuffer_t *fb)
{
    uint32_t width = fb->width;
    uint32_t height = fb->height;

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            const uint8_t *row = (uint8_t *)(fb->addr + (y * fb->pitch));

            const uint32_t bpp = fb->bpp;

            if (bpp == 32)
            {
                ((uint32_t *)row)[x] = color;
            }
            else if (bpp == 16)
            {
                ((uint16_t *)row)[x] = (uint16_t)color;
            }
        }
    }
}