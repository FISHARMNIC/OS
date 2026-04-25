#include <userspace/stdio.h>
#include <userspace/fs.h>

int main(int argc, char *argv[])
{
    fd_t fd;

    uint32_t err = ffind(&fd, "TEST");

    if(err)
    {
        printf("[ERROR] unable to find dir\n");
    }
    else
    {
    fd_t fds[10];

    uint32_t count = fls(fds, &fd, 10);

    for(uint32_t i = 0; i < count; i++)
    {
        FAT_filename_info_t* current = &fds[i].name;

        if(current->directory)
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