#include <userspace/stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc >= 1)
    {
        for (uint32_t i = 0; i < 10; i++)
        {
            printf("Thread [%s]: %d\n", argv[0], i);
        }
    }

    return 0;
}