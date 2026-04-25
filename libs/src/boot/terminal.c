#include <graphics.h>
#include <keyboard.h>
#include <sys/string.h>
#include <stdbool.h>
#include <fat.h>

static const char *prompt = "> ";

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

static bool terminal_builtin_command(const char *cmd)
{
    if (strcmp(cmd, "help") == 0)
    {
        tty_puts("Commands:\n\tclear\n\telftest\n\tfattest\n");
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
    else if (strcmp(cmd, "touchtest") == 0)
    {
        FAT_file_info_t info;
        FAT_read_entry_resp_t resp = fat32_find_file(&info, fat32_get_root(), "HELLO", "TXT", true);

        if (resp == FILE_FOUND)
        {
            tty_puts("Found file\n");
            uint8_t buffer[info.entry.fileSizeBytes];
            fat32_load_file(&info, buffer, info.entry.fileSizeBytes);
            tty_printf("Read [%d]: %s\n", info.entry.fileSizeBytes, buffer);
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
        tty_printf("\nGot: %s\n", buff);

        if (!terminal_builtin_command(buff))
        {
            tty_puts("Unknown command\n");
        }
    }
}