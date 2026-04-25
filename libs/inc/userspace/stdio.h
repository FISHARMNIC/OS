#ifndef STDIO_H
#define STDIO_H

#include <syscalls.h>
#include <stdint.h>
#include <files.h>
#include <stdarg.h>

// @todo make actual functions

static inline void puts(char *s)
{
    SYSCALL(SYSCALL_PUTS, s);
}

static inline void puti(uint32_t i)
{
    SYSCALL(SYSCALL_PUTI, i);
}

static inline void printf(const char *str, ...)
{
    va_list args;
    va_start(args, str);

    char str_s[2] = {0};

    bool fmtspec = false;

    uint32_t i = 0;
    while (str[i] != 0)
    {
        const char ch = str[i];
        if (fmtspec)
        {
            fmtspec = false;
            switch (ch)
            {
            case 'd':
                puti(va_arg(args, uint32_t));
                break;
            case 'c':
            {
                str_s[0] = (char) va_arg(args, uint32_t);
                puts(str_s);
                break;
            }
            case 's':
                puts(va_arg(args, char *));
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
            str_s[0] = ch;
            puts(str_s);
        }
        i++;
    }

    va_end(args);
}

#endif
