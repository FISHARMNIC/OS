#include <graphics.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/string.h>

static uint32_t tty_x = 0;
static uint32_t tty_y = 0;
tty_putch_handler_t tty_putch_handler;

uint32_t tty_cols(void)
{
    return graphics_context_active->fb->width / graphics_context_active->font.char_width;
}

uint32_t tty_rows(void)
{
    return graphics_context_active->fb->height / graphics_context_active->font.char_height;
}

void tty_clear()
{
    graphics_clear_screen(graphics_context_active->color_bg, graphics_context_active->fb);

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

static void tty_putch_default_handler(char c)
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
        else
        {
            tty_y--;
            tty_x = tty_cols() - 1;
        }
        graphics_draw_char((uint8_t)' ', tty_x * graphics_context_active->font.char_width, tty_y * graphics_context_active->font.char_height, graphics_context_active);
        return;
    }

    if (tty_x >= tty_cols())
    {
        tty_newline();
    }

    graphics_draw_char((uint8_t)c, tty_x * graphics_context_active->font.char_width, tty_y * graphics_context_active->font.char_height, graphics_context_active);
    tty_x++;

    if (tty_x >= tty_cols())
    {
        tty_newline();
    }
}

void tty_reset(void)
{
    tty_x = 0;
    tty_y = 0;
    tty_putch_handler = tty_putch_default_handler;
}

void tty_putch(char c)
{
    tty_putch_handler(c);
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
    bool colspec = false;

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
        else if(colspec)
        {
            colspec = false;
            switch(ch)
            {
                case 'b':
                    graphics_context_active->color_fg = COLOR_BLACK;
                    break;
                case 'w':
                    graphics_context_active->color_fg = COLOR_WHITE;
                    break;
                case 'r':
                    graphics_context_active->color_fg = COLOR_FORMAT_RGB(200, 50, 50);
                    break;
                case 'g':
                    graphics_context_active->color_fg = COLOR_FORMAT_RGB(50, 200, 50);
                    break;
                case 'u':
                    graphics_context_active->color_fg = COLOR_FORMAT_RGB(50, 50, 200);
                    break;
                case 'p':
                    graphics_context_active->color_fg = COLOR_FORMAT_RGB(200, 50, 200);
                    break;
                default:
                    break;
            }
        }
        else if (ch == '%')
        {
            fmtspec = true;
        }
        else if(ch == '\x1B')
        {
            // tty_puts("-COL-");
            colspec = true;
        }
        else
        {
            tty_putch(ch);
        }
        i++;
    }

    va_end(args);
}