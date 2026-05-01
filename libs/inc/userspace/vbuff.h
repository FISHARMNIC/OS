#include <syscalls.h>
#include <graphics.h>

static inline void vbuff(framebuffer_t *s)
{
    SYSCALL(SYSCALL_VBUFF, s);
}

static inline void vbuff_dispose(framebuffer_t *s)
{
    SYSCALL(SYSCALL_DISPOSEVBUFF, s);
}

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color, framebuffer_t *fb) // @todo move this to actual function
{
    if ((uint32_t)x >= fb->width || (uint32_t)y >= fb->height)
    {
        return;
    }

    const uint8_t *row = (uint8_t *)(fb->addr + (y * fb->pitch));

    const uint32_t bpp = fb->bpp;

    if (bpp == 32)
    {
        ((uint32_t *)row)[x] = color;
    }
    else if (bpp == 16)
    {
        ((uint16_t *)row)[x] = (uint16_t)color;
    }
}