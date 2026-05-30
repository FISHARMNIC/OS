#include "inc/wserver.h"
#include <userspace/vbuff.h>

static const uint8_t mouse_glyph[] = {
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100,
    0b11111110,
    0b11111100,
    0b11111000,
    0b11111000,
    0b11101100,
    0b11001100,
    0b10000110,
    0b00000110,
    0b00000011,
    0b00000011
};

void mouse_render(framebuffer_t *fb)
{
    uint32_t mouse_x = input.mouse.mouse_x < 0 ? 0 : (uint32_t)input.mouse.mouse_x;
    uint32_t mouse_y = input.mouse.mouse_y < 0 ? 0 : (uint32_t)input.mouse.mouse_y;

    for (uint32_t offy = 0; offy < 16; offy++)
    {
        for (uint32_t offx = 0; offx < 8; offx++)
        {
            if ((mouse_glyph[offy] & MASK(8 - 1 - offx)) != 0)
            {
                put_pixel(mouse_x + offx, mouse_y + offy, COLOR_BLACK, fb);
            }
        }
    }
}
