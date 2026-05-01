#include <userspace/events.h>
#include <userspace/stdio.h>
#include <stdbool.h>

volatile bool exit = false;

void kb_handler(uint8_t c, keyboard_event_t event)
{
    if(c == KEY_ENTER && event == KEYBOARD_EVENT_KEY_RELEASE)
    {
        exit = true;
    }
    else
    {
        printf("\tKeyboard %s event, keycode: %d\n", event == KEYBOARD_EVENT_KEY_PRESS ? "Press" : "Release", c);
    }
}

void click_handler(int32_t mouse_x, int32_t mouse_y, mouse_click_event_t mouse_edge_type)
{
    printf("\tMouse %s event at (%d, %d)\n", mouse_edge_type == MOUSE_EVENT_MOUSEDOWN ? "Down" : "Up", mouse_x, mouse_y);
}

void move_handler(int32_t mouse_x, int32_t mouse_y, int8_t dx, int8_t dy)
{
    printf("\tMouse Move event to (%d, %d) velocity (%d, %d)\n", mouse_x, mouse_y, dx, dy);
}

int main()
{
    handle_t kb_handle = user_events_add_keyboard(kb_handler);
    mouse_ev_handles_t move_handle = user_events_add_mouse(click_handler, move_handler);

    printf("Events attached... Hit [ENTER] to exit.\n");

    while(!exit);

    printf("\tExiting\n");

    user_events_remove(move_handle.click, move_handle.move, kb_handle);

    return 0;
}