#include <userspace/vbuff.h>

int main(int argc, char *argv[])
{
    framebuffer_t fb;

    vbuff(&fb);

    for (uint32_t y = 0; y < 256; y++)
    {
        for (uint32_t x = 0; x < 256; x++)
        {
            put_pixel(x, y, COLOR_FORMAT_RGB(x, y, x + y), &fb);
        }
    }

    vbuff_dispose(&fb);

    return 0;
}