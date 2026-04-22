#include <graphics.h>
#include <keyboard.h>
#include <sys/string.h>
#include <stdbool.h>

static const char *prompt = "> ";

void drawch(uint8_t c)
{
    if(c == KEY_BACKSPACE)
    {
        tty_putch('\b');
    }
    else if (c != KEY_ENTER)
    {
        tty_putch(keyboard_keycode_to_char(c));
    }
}

static bool terminal_builtin_command(const char *cmd)
{
    if (strcmp(cmd, "clear") == 0)
    {
        tty_clear();
        return true;
    }
    return false;
}


void terminal()
{
    keyboard_on_press_fn = drawch;

    char buff[100];
    while (1)
    {
        tty_puts(prompt);
        keyboard_gets(buff, sizeof(buff));
        tty_printf("\nGot: %s\n", buff);

        if(!terminal_builtin_command(buff))
        {
            tty_puts("Unknown command\n");
        }
    }
}