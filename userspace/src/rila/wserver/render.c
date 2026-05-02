#include "inc/wserver.h"
#include "inc/window.h"

#include <userspace/fs.h>
#include <userspace/malloc.h>
#include <userspace/stdio.h>
#include <userspace/vbuff.h>

static graphics_context_t ctx;
static framebuffer_t render_buffer_fb;
static uint8_t *render_buffer = NULLPTR;

void panic()
{
    printf("[RENDER PANIC] Unable to continue rendering. Halting.\n");
    while (1)
        ;
    // @todo
}

static void swap_buffers()
{
    uint8_t *front = (uint8_t *)framebuffer.addr;
    uint8_t *back = (uint8_t *)render_buffer_fb.addr;
    uint32_t size = framebuffer.pitch * framebuffer.height;

    for (uint32_t i = 0; i < size; i++)
    {
        front[i] = back[i];
    }
}

render_init_errors_t render_init()
{
    fd_t font_fd;
    uint32_t err = ffind(&font_fd, "SYS/FONT2.F16"); // @todo move to fonts/font.f16
    if (err)
    {
        return RIN_ERR_FONT;
    }

    uint32_t font_size = fsize(&font_fd);
    ctx.font = malloc(font_size);
    if (ctx.font == NULLPTR)
    {
        return RIN_ERR_FONT_MALLOC;
    }

    if (fread(&font_fd, (char *)ctx.font, font_size) != font_size)
    {
        free((uint8_t *)ctx.font);
        ctx.font = NULLPTR;
        return RIN_ERR_FONT_READ;
    }

    uint32_t render_buffer_size = framebuffer.pitch * framebuffer.height;
    render_buffer = malloc(render_buffer_size);
    if (render_buffer == NULLPTR)
    {
        free((uint8_t *)ctx.font);
        ctx.font = NULLPTR;
        return RIN_ERR_BUFFER_MALLOC;
    }

    render_buffer_fb = framebuffer;
    render_buffer_fb.addr = (uint32_t)render_buffer;

    ctx.color_fg = COLOR_BLACK;
    ctx.fb = &render_buffer_fb;

    err = desktop_init();
    if (err != RIN_OK)
    {
        free(render_buffer);
        free((uint8_t *)ctx.font);
        render_buffer = NULLPTR;
        ctx.font = NULLPTR;
        return err;
    }

    return RIN_OK;
}

void render_deinit()
{
    desktop_deinit();
    free((uint8_t *)ctx.font);
    free(render_buffer);
}

void render()
{
    clear_screen(BG_COLOR, &render_buffer_fb);

    desktop_render(&ctx);

    // window_render(&(window_t){
    //                   .x = 100,
    //                   .y = 100,
    //                   .event = NULLPTR,
    //                   .flags = 0,
    //                   .title = "TEST",
    //                   .width = 400,
    //                   .height = 300},
    //               0, 0, &ctx);
    // window_render(&(window_t){
    //                   .x = 120,
    //                   .y = 120,
    //                   .event = NULLPTR,
    //                   .flags = 0,
    //                   .title = "TEST",
    //                   .width = 400,
    //                   .height = 300},
    //               0, 0, &ctx);
    // window_render(&(window_t){
    //                   .x = 140,
    //                   .y = 140,
    //                   .event = NULLPTR,
    //                   .flags = 0,
    //                   .title = "TEST",
    //                   .width = 400,
    //                   .height = 300},
    //               0, 0, &ctx);

    mouse_render(&render_buffer_fb);

    swap_buffers();
}
