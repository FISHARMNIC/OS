#include <userspace/stdio.h>
#include <userspace/fs.h>

static void str_toupper(char *s)
{
    if (!s)
        return;

    while (*s)
    {
        if (*s >= 'a' && *s <= 'z')
            *s -= ('a' - 'A');
        s++;
    }
}

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        printf("[USAGE] cat path/to/file.txt\n");
    }
    else
    {
        char *dir = argv[0];
        str_toupper(dir);

        fd_t fd;
        uint32_t err = ffind(&fd, dir);

        if (err)
        {
            printf("[ERROR] Directory/file '%s' not found\n", dir);
        }
        else if (fd.name.directory)
        {
            printf("[ERROR] '%s' is a directory, not a file\n", dir);
        }
        else
        {
            uint32_t size = fsize(&fd);
            uint8_t buffer[size];
            fread(&fd, buffer, size);
            printf("%s\n", buffer);
        }
    }
    return 0;
}