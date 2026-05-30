#ifndef RILA_WINDOW_H
#define RILA_WINDOW_H

#include <graphics.h>
#include <stdint.h>

#define WINDOW_BAR_HEIGHT 20
#define WINDOW_RESIZE_AREA 10
#define WINDOW_MIN_WIDTH 20
#define WINDOW_MIN_HEIGHT 20

#define FLAGS_CLICK 0x1
#define FLAGS_MOVE 0x2
#define FLAGS_KEYP 0x4

typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    char *title;
    void *event;
    graphics_context_t window_ctx;
    uint8_t flags;
} window_t;

typedef struct
{
    window_t *ptr;
    window_t win;
    uint32_t x;
    uint32_t y;
    uint32_t outOfBar;
    uint32_t resize;
} window_selected_t;

typedef void window_event_fn(void *event);

void window_draw(window_t *window, int32_t rel_click_x, int32_t rel_click_y, graphics_context_t *ctx);
void window_render(window_t *window, int32_t rel_click_x, int32_t rel_click_y, graphics_context_t *ctx);
void window_render_all(int32_t click_x, int32_t click_y, graphics_context_t *ctx);

#endif
