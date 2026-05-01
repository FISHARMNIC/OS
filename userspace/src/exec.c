#include <userspace/fs.h>
#include <userspace/sys.h>
#include <userspace/stdio.h>

static void str_toupper(char *s)
{
    if (!s) return;

    while (*s)
    {
        if (*s >= 'a' && *s <= 'z')
            *s -= ('a' - 'A');
        s++;
    }
}

int main(int argc, char *argv[])
{
    fd_t fd;

    if (argc == 0)
    {
        printf("[USAGE] exec path/to/file.elf\n");
    }
    else
    {
        str_toupper(argv[0]);
        uint32_t err = ffind(&fd, argv[0]);

        if (err || fd.name.directory)
        {
            printf("[ERROR] Invalid file\n");
        }
        else
        {
            exec(&fd, argc - 1, &argv[1]);
        }
    }

    return 0;
}