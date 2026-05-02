#ifndef RILA_RENDER_H
#define RILA_RENDER_H

#include <stdint.h>

#include "launch.h"

#define LS_REFRESH_LIMIT 1000

#define BG_COLOR COLOR_FORMAT_RGB(176, 206, 255)

void render_desktop();

typedef enum {
    RIN_OK = 0,
    RIN_ERR_DESKTOP,
    RIN_ERR_FONT,
    RIN_ERR_FONT_MALLOC,
    RIN_ERR_FONT_READ,
    RIN_ERR_BUFFER_MALLOC,
    RIN_ERR_BITMAP,
    RIN_ERR_BITMAP_MALLOC,
    RIN_ERR_BITMAP_READ
} render_init_errors_t;

render_init_errors_t render_init();
void render_deinit();

void render();

#endif
