#include <stdbool.h>

#include <userspace/stdio.h>
#include <userspace/fs.h>
#include <userspace/malloc.h>

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

static uint32_t strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
        {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int main(int argc, char *argv[])
{
    fd_t fd;

    uint32_t err;

    bool root = false;
    if (argc == 0)
    {
        printf("Listing root\n");
        err = ffind(&fd, NULLPTR);
        root = true;
    }
    else
    {
        str_toupper(argv[0]);
        printf("Listing %s\n", argv[0]);
        err = ffind(&fd, argv[0]);
    }

    if (err)
    {
        printf("[ERROR] unable to find dir\n");
    }
    else if (!fd.name.directory && !root)
    {
        printf("[ERROR] not a directory\n");
    }
    else
    {
        bool include_dirs = true;
        bool include_files = true;
        bool show_extensions = true;

        uint32_t i = 1;
        while(argc >= 2)
        {
            char *filter = argv[i];
            if (strcmp(filter, "f") == 0)
            {
                include_dirs = false;
                // printf("files only\n");
            }
            else if (strcmp(filter, "d") == 0)
            {
                include_files = false;
                // printf("directories only\n");
            }
            else if(strcmp(filter, "noext") == 0)
            {
                show_extensions = false;
                // printf("hiding extensions\n");
            }
            else
            {
                printf("[ERROR] Unknown argument '%s'. Valid arguments are:\n\tf : Show only files\n\td : Show only directories\n\tnoext : Hide file extensions\n", filter);
                return 1;
            }

            argc--;
            i++;
        }

        uint32_t dirsize = fdirsize(&fd);
        fd_t *fds = malloc(dirsize * sizeof(fd_t));

        if (fds == NULLPTR)
        {
            printf("[ERROR] malloc failure\n");
            return 1;
        }

        uint32_t count = fls(fds, &fd, dirsize);

        for (uint32_t i = 0; i < count; i++)
        {
            FAT_filename_info_t *current = &fds[i].name;

            if (current->directory)
            {
                if (include_dirs)
                {
                    printf("\t%s/\n", current->name);
                }
            }
            else if (include_files)
            {
                if(!show_extensions)
                {
                    printf("\t%s\n", current->name);
                }
                else
                {
                    printf("\t%s.%s\n", current->name, current->name + current->extension_begin);
                }
            }
        }

        free(fds);
    }

    return 0;
}