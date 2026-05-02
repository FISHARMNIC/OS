#ifndef USER_VBUFF_H
#define USER_VBUFF_H

#include <stdbool.h>

#include <syscalls.h>
#include <graphics.h>

static inline void vbuff(framebuffer_t *s)
{
    SYSCALL(SYSCALL_VBUFF, s);
}

static inline void vbuff_dispose(framebuffer_t *s)
{
    SYSCALL(SYSCALL_DISPOSEVBUFF, s);
}

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color, framebuffer_t *fb) // @todo move this to actual function
{
    if (x >= fb->width || y >= fb->height)
    {
        return;
    }

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

static inline void fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, framebuffer_t *fb) // @todo move this to actual function
{
    for (uint32_t j = 0; j < height; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            put_pixel(x + i, y + j, color, fb);
        }
    }
}

static inline void clear_screen(uint32_t color, framebuffer_t *fb) // @todo move this to actual function
{
    fill_rect(0, 0, fb->width, fb->height, color, fb);
}

// @todo none of these should be inline

static inline void draw_glyph(const uint8_t *glyph, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    const uint8_t bpp = ctx->fb->bpp;
    const uint32_t pitch = ctx->fb->pitch;
    const uint32_t width = ctx->fb->width;
    const uint32_t height = ctx->fb->height;
    const uint32_t addr = ctx->fb->addr;

    const uint32_t color_fg = ctx->color_fg;
    // const uint32_t color_bg = ctx->color_bg;

    for (uint32_t offy = 0; offy < CHAR_HEIGHT; offy++)
    {
        for (uint32_t offx = 1; offx <= CHAR_WIDTH; offx++)
        {

            const uint32_t sx = x + offx;
            const uint32_t sy = y + offy;

            if ((uint32_t)sx >= width || (uint32_t)sy >= height)
            {
                continue;
            }

            // const uint32_t color = (glyph[offy] & MASK(CHAR_WIDTH - offx)) ? color_fg : color_bg;
            // const uint8_t *row = (uint8_t *)(addr + (sy * pitch));

            // if (bpp == 32) {
            //     ((uint32_t *)row)[sx] = color;
            // } else if (bpp == 16) {
            //     ((uint16_t *)row)[sx] = (uint16_t)color;
            // }

            bool draw = (glyph[offy] & MASK(CHAR_WIDTH - offx));
            if (draw)
            {
                const uint8_t *row = (uint8_t *)(addr + (sy * pitch));

                if (bpp == 32)
                {
                    ((uint32_t *)row)[sx] = color_fg;
                }
                else if (bpp == 16)
                {
                    ((uint16_t *)row)[sx] = (uint16_t)color_fg;
                }
            }
        }
    }
}

static inline void draw_char(uint8_t character, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    const uint8_t *glyph = ctx->font + (character * CHAR_HEIGHT);

    draw_glyph(glyph, x, y, ctx);
}

static inline void draw_string(char *string, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    while (*string)
    {
        draw_char((uint8_t)*string, x, y, ctx);
        x += CHAR_WIDTH;
        string++;
    }
}

#endif