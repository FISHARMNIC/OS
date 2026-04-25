#include <userspace/stdio.h>
#include <userspace/fs.h>

int main()
{
    fd_t fd;

    uint32_t err = ffind(&fd, "TEST/HELLO.TXT");

    if(err)
    {
        puts("Unable to find file\n");
    }
    else
    {
        uint32_t size = fsize(&fd);

        printf("File size: %d\n", size);

        // uint8_t buffer[size];

        // fread(&fd, buffer, size);

        // puts(buffer);
    }

    return 0;
}