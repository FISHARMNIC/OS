#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <boot.h>

#define CHAR_HEIGHT 16
#define CHAR_WIDTH 8

#define MASK(offset) (1U << (offset))

#define COLOR_RED_MAX 255
#define COLOR_GREEN_MAX 255
#define COLOR_BLUE_MAX 255

#define COLOR_FORMAT_RGB(r, g, b) ((uint32_t)(((r) << 16) | ((g) << 8) | (b)))


#define COLOR_WHITE COLOR_FORMAT_RGB(COLOR_RED_MAX, COLOR_GREEN_MAX, COLOR_BLUE_MAX)
#define COLOR_BLACK COLOR_FORMAT_RGB(0, 0, 0)

typedef struct
{
    uint32_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
} framebuffer_t;

typedef struct
{
    uint32_t color_fg;
    uint32_t color_bg;
    const uint8_t *font;
    const framebuffer_t *fb;
} graphics_context_t;

typedef void (*tty_putch_handler_t)(char c);

extern const uint8_t _binary_FONT_F16_start[];

extern framebuffer_t graphics_fb_default;
extern framebuffer_t *graphics_fb_active;

extern graphics_context_t graphics_context_default;

/**
 * @brief Sets up a frame buffer given boot record info
 */
void graphics_init_fb(framebuffer_t *fb, multiboot_info_t *mbi);

/**
 * @brief Sets up a render context which holds color, font, and framebuffer info
 */
void graphics_init_context(graphics_context_t *context, framebuffer_t *fb, const uint8_t *font, uint32_t color_fg, uint32_t color_bg);

void graphics_draw_glyph(const uint8_t *glyph, uint32_t x, uint32_t y, graphics_context_t *ctx);
void graphics_draw_char(uint8_t character, uint32_t x, uint32_t y, graphics_context_t *ctx);

void graphics_draw_string(char *string, uint32_t x, uint32_t y, graphics_context_t *ctx);

uint32_t tty_cols(void);
uint32_t tty_rows(void);

void tty_clear();
void tty_reset();
void tty_putch(char c);
void tty_puts(const char *str);
void tty_puti(int32_t value);
void tty_printf(const char *str, ...);

void graphics_get_buffer(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t *saveBuffer, graphics_context_t *ctx);
void graphics_draw_buffer(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t *saveBuffer, graphics_context_t *ctx);

#endif
