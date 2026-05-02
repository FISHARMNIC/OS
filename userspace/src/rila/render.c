#include "inc/render.h"
#include "libs/bmp.h"
#include <userspace/fs.h>
#include <userspace/malloc.h>
#include <userspace/stdio.h>
#include <userspace/vbuff.h>

static fd_t desktop_fd;
static graphics_context_t ctx;
static framebuffer_t render_buffer_fb;
static uint8_t *render_buffer = NULLPTR;
static uint8_t *folder_bmp = NULLPTR;
static uint32_t folder_bmp_size = 0;

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
    0b11101100,
    0b10000110,
    0b00000110,
    0b00000011,
    0b00000011
};

void panic()
{
    printf("[RENDER PANIC] Unable to continue rendering. Halting.\n");
    while (1);
    // @todo
}

static fd_t *desk_infos = NULLPTR;
void render_desktop()
{
    static uint32_t ls_refresh = LS_REFRESH_LIMIT; // @todo this should instead be checked with the last modified date on the folder
    static uint32_t size = 0;

    if (ls_refresh == LS_REFRESH_LIMIT)
    {
        ls_refresh = 0;
        size = fdirsize(&desktop_fd);

        if (desk_infos != NULLPTR)
        {
            free(desk_infos);
        }

        desk_infos = malloc(size * sizeof(fd_t));

        if (desk_infos == NULLPTR)
        {
            panic();
        }

        fls(desk_infos, &desktop_fd, size);
    }

    uint32_t x = 0;
    for (uint32_t i = 0; i < size; i++)
    {
        char* name = desk_infos[i].name.name;

        if(!(name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))))
        {
            rila_bmp_draw(folder_bmp, folder_bmp_size, 30 + x * 60, 30, &ctx);
            draw_string(name, 30 + x * 60, 80, &ctx);
            x++;
        }
    }

    ls_refresh++;
}

static void render_mouse()
{
    uint32_t mouse_x = input.mouse.mouse_x < 0 ? 0 : (uint32_t)input.mouse.mouse_x;
    uint32_t mouse_y = input.mouse.mouse_y < 0 ? 0 : (uint32_t)input.mouse.mouse_y;

    for (uint32_t offy = 0; offy < CHAR_HEIGHT; offy++)
    {
        for (uint32_t offx = 0; offx < CHAR_WIDTH; offx++)
        {
            if ((mouse_glyph[offy] & MASK(CHAR_WIDTH - 1 - offx)) != 0)
            {
                put_pixel(mouse_x + offx, mouse_y + offy, COLOR_BLACK, &render_buffer_fb);
            }
        }
    }
}

static void swap_buffers()
{
    uint8_t *front = (uint8_t*) framebuffer.addr;
    uint8_t *back = (uint8_t*) render_buffer_fb.addr;
    uint32_t size = framebuffer.pitch * framebuffer.height;

    for (uint32_t i = 0; i < size; i++)
    {
        front[i] = back[i];
    }
}

render_init_errors_t render_init()
{
    uint32_t err = ffind(&desktop_fd, NULLPTR);
    if (err)
    {
        return RIN_ERR_DESKTOP;
    }

    fd_t font_fd;
    err = ffind(&font_fd, "BOOT/FONT.F16"); // @todo move to fonts/font.f16
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

    if (fread(&font_fd, (char*) ctx.font, font_size) != font_size)
    {
        free((uint8_t*) ctx.font);
        ctx.font = NULLPTR;
        return RIN_ERR_FONT_READ;
    }

    uint32_t render_buffer_size = framebuffer.pitch * framebuffer.height;
    render_buffer = malloc(render_buffer_size);
    if (render_buffer == NULLPTR)
    {
        free((uint8_t*) ctx.font);
        ctx.font = NULLPTR;
        return RIN_ERR_BUFFER_MALLOC;
    }

    render_buffer_fb = framebuffer;
    render_buffer_fb.addr = (uint32_t) render_buffer;

    fd_t folder_fd;
    err = ffind(&folder_fd, "SYS/FOLD.BMP");
    if (err)
    {
        free(render_buffer);
        free((uint8_t*) ctx.font);

        render_buffer = NULLPTR;
        ctx.font = NULLPTR;
        return RIN_ERR_BITMAP;
    }

    folder_bmp_size = fsize(&folder_fd);
    folder_bmp = malloc(folder_bmp_size);
    if (folder_bmp == NULLPTR)
    {
        free(render_buffer);
        free((uint8_t*) ctx.font);

        render_buffer = NULLPTR;
        ctx.font = NULLPTR;
        return RIN_ERR_BITMAP_MALLOC;
    }

    if (fread(&folder_fd, (char*) folder_bmp, folder_bmp_size) != folder_bmp_size)
    {
        free(folder_bmp);
        free(render_buffer);
        free((uint8_t*) ctx.font);

        folder_bmp = NULLPTR;
        render_buffer = NULLPTR;
        ctx.font = NULLPTR;
        return RIN_ERR_BITMAP_READ;
    }

    ctx.color_fg = COLOR_BLACK;
    // ctx.color_bg = COLOR_BLACK;
    ctx.fb = &render_buffer_fb;

    return RIN_OK;
}

void render_deinit()
{
    free((uint8_t*) ctx.font);
    free(folder_bmp);
    free(render_buffer);
    free(desk_infos);
}

void render()
{
    clear_screen(BG_COLOR, &render_buffer_fb);
    render_desktop();
    render_mouse();
    swap_buffers();
}
