#include <graphics.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/string.h>

static uint32_t tty_x = 0;
static uint32_t tty_y = 0;

static uint32_t tty_cols(void)
{
    return graphics_fb_active->width / CHAR_WIDTH;
}

static uint32_t tty_rows(void)
{
    return graphics_fb_active->height / CHAR_HEIGHT;
}

void tty_clear(void)
{
    const uint8_t bpp = graphics_fb_active->bpp;
    const uint32_t width = graphics_fb_active->width;
    const uint32_t height = graphics_fb_active->height;
    const uint32_t pitch = graphics_fb_active->pitch;
    const uint32_t addr = graphics_fb_active->addr;
    const uint32_t color = graphics_context_default.color_bg;

    for (uint32_t y = 0; y < height; ++y)
    {
        uint8_t *row = (uint8_t *)(addr + (y * pitch));
        for (uint32_t x = 0; x < width; ++x)
        {
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

    tty_reset();
}

void tty_reset(void)
{
    tty_x = 0;
    tty_y = 0;
}

static void tty_newline(void)
{
    tty_x = 0;
    tty_y++;

    if (tty_y >= tty_rows())
    {
        tty_clear();
    }
}

void tty_putch(char c)
{
    if (c == '\n')
    {
        tty_newline();
        return;
    }
    else if(c == '\t')
    {
        tty_puts("  ");
        return;
    }
    else if (c == '\b')
    {
        if (tty_x > 0)
        {
            tty_x--;
        }
        graphics_draw_char((uint8_t)' ', tty_x * CHAR_WIDTH, tty_y * CHAR_HEIGHT, &graphics_context_default);
        return;
    }

    if (tty_x >= tty_cols())
    {
        tty_newline();
    }

    graphics_draw_char((uint8_t)c, tty_x * CHAR_WIDTH, tty_y * CHAR_HEIGHT, &graphics_context_default);
    tty_x++;

    if (tty_x >= tty_cols())
    {
        tty_newline();
    }
}

void tty_puts(const char *str)
{
    while (*str != '\0')
    {
        tty_putch(*str++);
    }
}

void tty_puti(int32_t value)
{
    char buf[12];
    uint32_t n;
    uint32_t idx = 0;

    if (value == 0)
    {
        tty_putch('0');
        return;
    }

    if (value < 0)
    {
        tty_putch('-');
        n = (uint32_t)(-(value + 1)) + 1U;
    }
    else
    {
        n = (uint32_t)value;
    }

    while (n > 0U)
    {
        buf[idx++] = (char)('0' + (n % 10U));
        n /= 10U;
    }

    while (idx > 0U)
    {
        tty_putch(buf[--idx]);
    }
}

void tty_printf(const char *str, ...)
{
    va_list args;
    va_start(args, str);

    bool fmtspec = false;

    uint32_t i = 0;
    while (str[i] != 0)
    {
        const char ch = str[i];
        if (fmtspec)
        {
            fmtspec = false;
            switch(ch)
            {
                case 'd':
                    tty_puti(va_arg(args, uint32_t));
                    break;
                case 'c':
                    tty_putch(va_arg(args, uint32_t));
                    break;
                case 's':
                    tty_puts(va_arg(args, char*));
                    break;
                default:
                    break;
            }
        }
        else if (ch == '%')
        {
            fmtspec = true;
        }
        else
        {
            tty_putch(ch);
        }
        i++;
    }

    va_end(args);
}