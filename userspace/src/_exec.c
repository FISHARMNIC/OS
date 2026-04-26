#include <userspace/fs.h>
#include <userspace/sys.h>
#include <userspace/stdio.h>

int main(int argc, char *argv[])
{
    fd_t fd;

    uint32_t err = ffind(&fd, "BIN/LS.ELF");

    if(err)
    {
        printf("[ERROR] Invalid file\n");
    }
    else
    {
        exec(&fd, 0, 0);
    }

    return 0;
}