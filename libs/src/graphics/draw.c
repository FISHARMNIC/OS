#include <graphics.h>
#include <boot.h>

void graphics_draw_char(uint8_t character, uint32_t x, uint32_t y, graphics_context_t* ctx)
{
    const uint8_t *glyph = ctx->font + (character * CHAR_HEIGHT);

    const uint8_t bpp = ctx->fb->bpp;
    const uint32_t pitch = ctx->fb->pitch;
    const uint32_t width = ctx->fb->width;
    const uint32_t height = ctx->fb->height;
    const uint32_t addr = graphics_fb_active->addr;

    const uint32_t color_fg = ctx->color_fg;
    const uint32_t color_bg = ctx->color_bg;

    for (uint32_t offy = 0; offy < CHAR_HEIGHT; offy++) {
        for (uint32_t offx = 1; offx <= CHAR_WIDTH; offx++) {

            const uint32_t sx = x + offx;
            const uint32_t sy = y + offy;

            if ((uint32_t)sx >= width || (uint32_t)sy >= height) {
                continue;
            }

            const uint32_t color = (glyph[offy] & MASK(CHAR_WIDTH - offx)) ? color_fg : color_bg;
            const uint8_t *row = (uint8_t *)(addr + (sy * pitch));

            if (bpp == 32) {
                ((uint32_t *)row)[sx] = color;
            } else if (bpp == 16) {
                ((uint16_t *)row)[sx] = (uint16_t)color;
            }
        }
    }
}

void graphics_draw_string(char* string, uint32_t x, uint32_t y, graphics_context_t* ctx)
{
    while(*string)
    {
        graphics_draw_char((uint8_t)*string, x, y, ctx);
        x += CHAR_WIDTH;
        string++;
    }
}