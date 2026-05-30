#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <boot.h>

#define MASK(offset) (1U << (offset))

#define COLOR_RED_MAX 255
#define COLOR_GREEN_MAX 255
#define COLOR_BLUE_MAX 255

#define COLOR_FORMAT_RGB(r, g, b) ((uint32_t)(((r) << 16) | ((g) << 8) | (b)))


#define COLOR_WHITE COLOR_FORMAT_RGB(COLOR_RED_MAX, COLOR_GREEN_MAX, COLOR_BLUE_MAX)
#define COLOR_BLACK COLOR_FORMAT_RGB(0, 0, 0)

#define TTY_COLOR_ESCAPE "\x1B"
#define TC_BLACK TTY_COLOR_ESCAPE"b"
#define TC_WHITE TTY_COLOR_ESCAPE"w"
#define TC_RED TTY_COLOR_ESCAPE"r"
#define TC_GREEN TTY_COLOR_ESCAPE"g"
#define TC_BLUE TTY_COLOR_ESCAPE"u"
#define TC_PURP TTY_COLOR_ESCAPE"p"

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
    uint32_t char_width;
    uint32_t char_height;
    const uint8_t *ptr;
} font_info_t;

typedef struct
{
    font_info_t font;
    uint32_t color_fg;
    uint32_t color_bg;
    
    const framebuffer_t *fb;
} graphics_context_t;

typedef void (*tty_putch_handler_t)(char c);

extern const uint8_t _binary_FONT_F16_start[];

extern framebuffer_t graphics_fb_default;

extern graphics_context_t graphics_context_default;
extern graphics_context_t *graphics_context_active;

/**
 * @brief Sets the active graphics context
 */
void graphics_set_active_context(graphics_context_t *ctx);

/**
 * @brief Sets up a frame buffer given boot record info
 */
void graphics_init_fb(framebuffer_t *fb, multiboot_info_t *mbi);

/**
 * @brief Sets up a render context which holds color, font, and framebuffer info
 */
void graphics_init_context(graphics_context_t* context, framebuffer_t* fb, const font_info_t font, uint32_t color_fg, uint32_t color_bg);

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
void graphics_clear_screen(uint32_t color, const framebuffer_t *fb);



#endif
