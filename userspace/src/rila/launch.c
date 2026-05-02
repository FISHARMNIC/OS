#include "inc/launch.h"
#include "inc/server.h"

framebuffer_t framebuffer;
user_input_t input;

static void mouse_move_handler(int32_t mx, int32_t my, int8_t dx, int8_t dy)
{
    input.mouse.mouse_x = mx;
    input.mouse.mouse_y = my;
    
    input.mouse.dx = dx;
    input.mouse.dy = dy;
}

static void mouse_click_handler(int32_t mx, int32_t my, mouse_click_event_t et)
{
    input.mouse.mouse_x = mx;
    input.mouse.mouse_y = my;
    input.mouse.mouse_edge_type = et;
}

static void keyboard_key_handler(uint8_t sc, keyboard_event_t type)
{
    input.keyboard.sc = sc;
    input.keyboard.type = type;
}

static mouse_ev_handles_t mhandles;
static handle_t khandle;

void init()
{
    vbuff(&framebuffer);

    mhandles = user_events_add_mouse(mouse_click_handler, mouse_move_handler);
    khandle = user_events_add_keyboard(keyboard_key_handler);
}

void deinit()
{
    user_events_remove(mhandles.click, mhandles.move, khandle);

    vbuff_dispose(&framebuffer);
}

int main()
{   
    init();

    server();

    deinit();

    return 0;
}