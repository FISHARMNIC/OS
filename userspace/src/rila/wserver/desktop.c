#include "inc/wserver.h"
#include "../libs/inc/bmp.h"
#include <userspace/fs.h>
#include <userspace/malloc.h>

static fd_t desktop_fd;
static fd_t *desk_infos = NULLPTR;
static uint8_t *folder_bmp = NULLPTR;
static uint32_t folder_bmp_size = 0;

render_init_errors_t desktop_init()
{
    uint32_t err = ffind(&desktop_fd, NULLPTR);
    if (err)
    {
        return RIN_ERR_DESKTOP;
    }

    fd_t folder_fd;
    err = ffind(&folder_fd, "SYS/FOLD.BMP");
    if (err)
    {
        return RIN_ERR_BITMAP;
    }

    folder_bmp_size = fsize(&folder_fd);
    folder_bmp = malloc(folder_bmp_size);
    if (folder_bmp == NULLPTR)
    {
        return RIN_ERR_BITMAP_MALLOC;
    }

    if (fread(&folder_fd, (char*) folder_bmp, folder_bmp_size) != folder_bmp_size)
    {
        free(folder_bmp);
        folder_bmp = NULLPTR;
        return RIN_ERR_BITMAP_READ;
    }

    return RIN_OK;
}

void desktop_deinit()
{
    free(folder_bmp);
    free(desk_infos);
}

void desktop_render(graphics_context_t *ctx)
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
            rila_bmp_draw(folder_bmp, folder_bmp_size, 30 + x * 60, 30, ctx);
            draw_string(name , 35 + x * 60, 80, ctx);
            x++;
        }
    }

    ls_refresh++;
}
