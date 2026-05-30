#include "inc/wserver.h"
#include <userspace/vbuff.h>

static window_selected_t wasClicked;

static uint32_t window_bar_color(window_t *window)
{
    if (wasClicked.ptr == window)
    {
        return COLOR_WHITE;
    }

    return COLOR_FORMAT_RGB(110, 170, 110);
}

static void draw_bordered_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, uint32_t border_color, uint32_t border_width, framebuffer_t *fb)
{
    fill_rect(x, y, width, height, color, fb);

    for (uint32_t i = 0; i < border_width; i++)
    {
        fill_rect(x + i, y + i, width - i * 2, 1, border_color, fb);
        fill_rect(x + i, y + height - 1 - i, width - i * 2, 1, border_color, fb);
        fill_rect(x + i, y + i, 1, height - i * 2, border_color, fb);
        fill_rect(x + width - 1 - i, y + i, 1, height - i * 2, border_color, fb);
    }
}

static void draw_transparent_char(uint8_t character, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    const uint8_t *glyph = ctx->font.ptr + (character * ctx->font.char_height);

    for (uint32_t offy = 0; offy < ctx->font.char_height; offy++)
    {
        for (uint32_t offx = 0; offx < ctx->font.char_width; offx++)
        {
            if ((glyph[offy] & MASK(ctx->font.char_width - 1 - offx)) != 0)
            {
                put_pixel(x + offx, y + offy, ctx->color_fg, (framebuffer_t*)ctx->fb);
            }
        }
    }
}

static void draw_transparent_string(char *string, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    while (*string)
    {
        draw_transparent_char((uint8_t)*string, x, y, ctx);
        x += ctx->font.char_width;
        string++;
    }
}

static void widget_draw_all(window_t *window, int32_t rel_click_x, int32_t rel_click_y)
{
    (void)window;
    (void)rel_click_x;
    (void)rel_click_y;
}

static void window_events_trigger_expose(window_t *window)
{
    (void)window;
}

void window_draw(window_t *window, int32_t rel_click_x, int32_t rel_click_y, graphics_context_t *ctx)
{
    uint32_t x = window->x;
    uint32_t y = window->y;
    uint32_t width = window->width;
    uint32_t height = window->height;
    uint32_t saved_fg = ctx->color_fg;

    ctx->color_fg = COLOR_BLACK;

    draw_bordered_rect(x, y, width, WINDOW_BAR_HEIGHT, window_bar_color(window), COLOR_BLACK, 1, (framebuffer_t*)ctx->fb);
    draw_transparent_string("- + X", x + width - 50, y + 2, ctx);
    draw_transparent_string(window->title, x + 5, y + 2, ctx);
    draw_bordered_rect(x, y + WINDOW_BAR_HEIGHT - 1, width, height, /*window->window_ctx.color_fg*/ COLOR_FORMAT_RGB(73, 177, 230), COLOR_BLACK, 1, (framebuffer_t*)ctx->fb);
    draw_bordered_rect(x + width - WINDOW_RESIZE_AREA, y + WINDOW_BAR_HEIGHT + height - WINDOW_RESIZE_AREA - 1, WINDOW_RESIZE_AREA, WINDOW_RESIZE_AREA, COLOR_FORMAT_RGB(160, 160, 160), COLOR_BLACK, 1, (framebuffer_t*)ctx->fb);

    ctx->color_fg = saved_fg;

    widget_draw_all(window, rel_click_x, rel_click_y);
}

void window_render(window_t *window, int32_t rel_click_x, int32_t rel_click_y, graphics_context_t *ctx)
{
    window_draw(window, rel_click_x, rel_click_y, ctx);
    window_events_trigger_expose(window);
}

void window_render_all(int32_t click_x, int32_t click_y, graphics_context_t *ctx)
{
    (void)click_x;
    (void)click_y;
    (void)ctx;
}
