#include <graphics.h>

void graphics_get_buffer(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t* saveBuffer, graphics_context_t* ctx)
{
    const uint32_t swidth = ctx->fb->width;
    uint32_t *addr = &(((uint32_t*) ctx->fb->addr)[x + y * swidth]);

    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            saveBuffer[j] = addr[j]; 
        }
        saveBuffer += width;
        addr += swidth;
    }
}

void graphics_draw_buffer(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t* saveBuffer, graphics_context_t* ctx)
{
    const uint32_t swidth = ctx->fb->width;
    const uint32_t sheight = ctx->fb->height;

    uint32_t *addr = &(((uint32_t*) ctx->fb->addr)[x + y * swidth]);
    uint32_t *end =  &(((uint32_t*) ctx->fb->addr)[swidth + sheight * swidth]);

    for (uint32_t i = 0; i < height; i++) {
        
        for (uint32_t j = 0; j < width; j++) {
            if(&addr[j] >= end)
            {
                continue;
            }
            addr[j] = saveBuffer[j]; 
        }
        saveBuffer += width;
        addr += swidth;
    }
}