#include <graphics.h>
#include <keyboard.h>
#include <sys/string.h>
#include <stdbool.h>
#include <fat.h>
#include <files.h>
#include <elf.h>

static const char *prompt = "> ";

#define user_stack_size 4096
uint8_t user_stack_[user_stack_size] __attribute__((aligned(user_stack_size)));


void drawch(uint8_t c)
{
    if (c == KEY_BACKSPACE)
    {
        tty_putch('\b');
    }
    else if (c != KEY_ENTER)
    {
        tty_putch(keyboard_keycode_to_char(c));
    }
}

static bool terminal_builtin_command(const char *cmd, char *save)
{
    /*
    resp = strtok_r(NULLPTR, " ", &save);
    */

    if (strcmp(cmd, "help") == 0)
    {
        tty_puts("Commands:\n\tclear\n\ttouch\n\texec\n\telftest\n\tfattest\n");
        return true;
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        tty_clear();
        return true;
    }
    else if (strcmp(cmd, "elftest") == 0)
    {
        elftest(terminal);
        return true;
    }
    else if (strcmp(cmd, "fattest") == 0)
    {
        fattest();
        return true;
    }
    else if (strcmp(cmd, "touch") == 0)
    {
        char *dir = strtok_r(NULLPTR, " ", &save);
        str_toupper(dir);

        fd_t fd;
        uint32_t err = file_find(&fd, dir);

        if (err)
        {
            tty_printf("[ERROR] File '%s' not found\n", dir);
        }
        else
        {
            uint32_t size = file_size(&fd);
            uint8_t buffer[size];
            file_read(&fd, buffer, size);
            tty_printf("%s\n", buffer);
        }

        return true;
    }
    else if (strcmp(cmd, "exec") == 0)
    {
        char *dir = strtok_r(NULLPTR, " ", &save);
        str_toupper(dir);

        fd_t fd;
        uint32_t err = file_find(&fd, dir);

        if (err)
        {
            tty_printf("[ERROR] File '%s' not found\n", dir);
        }
        else
        {
            uint32_t size = file_size(&fd);
            uint8_t buffer[size];
            file_read(&fd, buffer, size);

            elf_exec(buffer, size, user_stack_, user_stack_size, terminal);
        }

        return true;
    }
    else
    {
        return false;
    }
}

void terminal()
{
    keyboard_on_press_fn = drawch;

    char buff[100];
    while (1)
    {
        tty_puts(prompt);
        keyboard_gets(buff, sizeof(buff));
        // tty_printf("\nGot: %s\n", buff);

        char *save;
        char *resp = strtok_r(buff, " ", &save);

        tty_putch('\n');

        bool valid = terminal_builtin_command(resp, save);

        if (valid)
        {
            tty_putch('\n');
        }
        else
        {
            tty_printf("Unknown command '%s'\n", resp);
        }
    }
}