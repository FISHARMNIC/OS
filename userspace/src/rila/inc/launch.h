#ifndef RILA_LAUNCH_H
#define RILA_LAUNCH_H

#include <userspace/vbuff.h>
#include <userspace/events.h>

typedef struct
{
    int32_t mouse_x;
    int32_t mouse_y;
    int8_t dx;
    int8_t dy;
    mouse_click_event_t mouse_edge_type;
} mouse_info_t;

typedef struct
{
    uint8_t sc;
    keyboard_event_t type;
} kb_info_t;

typedef struct
{
    mouse_info_t mouse;
    kb_info_t keyboard;
} user_input_t;

void init();

extern framebuffer_t framebuffer;
extern user_input_t input;

#endif