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
        uint8_t buffer[size];

        uint32_t bytes_read = fread(&fd, buffer, size);
        
        printf("File contents[%d]: '%s'\n", bytes_read, buffer);
    }

    return 0;
}