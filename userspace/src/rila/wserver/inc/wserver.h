#ifndef RILA_WSERVER_H
#define RILA_WSERVER_H

#include "../../inc/render.h"
#include "window.h"

void panic();

render_init_errors_t desktop_init();
void desktop_deinit();

void desktop_render(graphics_context_t *ctx);
void mouse_render(framebuffer_t *fb);

#endif
