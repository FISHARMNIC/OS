#include <userspace/stdio.h>
#include <userspace/fs.h>

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

    uint32_t err;
    if (argc == 0)
    {
        printf("LS from root\n");
        err = ffind(&fd, NULLPTR);
    }
    else
    {
        str_toupper(argv[0]);
        printf("LS from dir: %s\n", argv[0]);
        err = ffind(&fd, argv[0]);
    }

    if (err)
    {
        printf("[ERROR] unable to find dir\n");
    }
    else
    {
        fd_t fds[10];

        uint32_t count = fls(fds, &fd, 10);

        for (uint32_t i = 0; i < count; i++)
        {
            FAT_filename_info_t *current = &fds[i].name;

            if (current->directory)
            {
                printf("%s/\n", current->name);
            }
            else
            {
                printf("%s.%s\n", current->name, current->name + current->extension_begin);
            }
        }
    }

    return 0;
}