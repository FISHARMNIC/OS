#include <graphics.h>
#include <keyboard.h>
#include <sys/string.h>
#include <stdbool.h>
#include <fat.h>
#include <files.h>
#include <elf.h>

static const char *prompt = "> ";

#define PWD_SIZE 100

char pwd_last[PWD_SIZE];
char pwd[PWD_SIZE];

void drawch(uint8_t c)
{
    static uint32_t n = 0;

    if (c == KEY_BACKSPACE)
    {
        if(n > 0)
        {
            tty_putch('\b');
        }
    }
    else if (c != KEY_ENTER)
    {
        tty_putch(keyboard_keycode_to_char(c));
        n++;
    }
    else
    {
        n = 0;
    }
}

static bool terminal_bin_cmd(char *cmd, char *save)
{
    static fd_t infos[20];
    fd_t bin;
    file_find(&bin, "BIN");

    str_toupper(cmd);

    uint32_t count = files_ls(infos, 20, bin.cluster);

    for (uint32_t i = 0; i < count; i++)
    {
        FAT_filename_info_t name = infos[i].name;

        if (!name.directory && strcmp(name.name + name.extension_begin, "ELF") == 0 && strcmp(cmd, name.name) == 0)
        {

            uint32_t size = file_size(&infos[i]);
            uint8_t buffer[size];
            file_read(&infos[i], buffer, size);

            char *args[10]; // @todo make this dynamic or do some better way

            i = 0;

            while (i < 10)
            {
                char *arg = strtok_r(NULLPTR, " ", &save);
                if (arg == NULLPTR)
                {
                    break;
                }
                else
                {
                    // tty_printf("ARG %s\n", arg);
                    args[i] = arg;
                }
                i++;
            }

            elf_exec(buffer, size, user_stack_glob, user_stack_size, i, args);

            return true;
        }
    }

    return false;
}

static bool terminal_builtin_command(const char *cmd, char *save)
{
    /*
    resp = strtok_r(NULLPTR, " ", &save);
    */

    if (strcmp(cmd, "help") == 0)
    {
        tty_puts("Built in commands:\n\tclear\n\texec\n");
        tty_puts("BIN commands:\n");
        terminal_bin_cmd("LS", "BIN");
        return true;
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        tty_clear();
        return true;
    }
    else if (strcmp(cmd, "exec") == 0)
    {
        char *dir = strtok_r(NULLPTR, " ", &save);
        if (dir == NULLPTR)
        {
            tty_printf("[USAGE] exec path/to/file.elf\n", dir);
            return true;
        }

        str_toupper(dir);

        fd_t fd;
        uint32_t err = file_find(&fd, dir);

        if (err)
        {
            tty_printf("[ERROR] File '%s' not found\n", dir);
        }
        else if (fd.name.directory)
        {
            tty_printf("[ERROR] '%s' is a directory, not a file\n", dir);
        }
        else
        {
            uint32_t size = file_size(&fd);
            uint8_t buffer[size];
            file_read(&fd, buffer, size);

            char *args[10]; // @todo make this dynamic or do some better way

            uint32_t i = 0;

            while (i < 10)
            {
                char *arg = strtok_r(NULLPTR, " ", &save);
                if (arg == NULLPTR)
                {
                    break;
                }
                else
                {
                    args[i] = arg;
                }
                i++;
            }

            elf_exec(buffer, size, user_stack_glob, user_stack_size, i, args);
        }

        return true;
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        // @todo ../ still appends to pwd string and just makes it longer until overflow + slowdown. Make it remove last if ../
        // @todo similar for ./

        // @todo do bounds check on pwd size
        char *dir = strtok_r(NULLPTR, " ", &save);
        if(dir == NULLPTR)
        {
            tty_printf("[USAGE] cd path\n");
        }
        else
        {
            str_toupper(dir);

            memcpy(pwd_last, pwd, PWD_SIZE);

            if(pwd[0] != 0 && pwd[strlen(pwd) - 1] != '/')
            {
                strcat(pwd, "/");
            }
            strcat(pwd, dir);

            fd_t fd;
            uint32_t err = file_find(&fd, pwd);
            if(err)
            {
                tty_printf("[ERROR] Path [%s] does not exist\n", pwd);
                memcpy(pwd, pwd_last, PWD_SIZE);
            }
            else
            {
                tty_printf("pwd: %s\n", pwd);
            }
        }
        return true;
    }
    else if(strcmp(cmd, "pwd") == 0)
    {
        tty_printf("pwd: %s\n", pwd);
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
            // tty_putch('\n');
        }
        else if (resp != NULLPTR)
        {
            valid = terminal_bin_cmd(resp, save);
            if (!valid)
            {
                tty_printf("Unknown command '%s'. Run 'help' for a list of commands\n", resp);
            }
        }
    }
}