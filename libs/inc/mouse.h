#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <cpu.h>
#include <ports.h>

#define MOUSE_PORT_1 0x60
#define MOUSE_PORT_2 0x64

#define MOUSE_PACKET_HEADER 0xD4
#define MOUSE_ENABLE_PACKET_STREAMING 0xF4
#define MOUSE_ACK 0xFA
#define MOUSE_ENABLE_INT 0x20

#define MOUSE_BUTTONS_MASK 0b00000111

#define MOUSE_LEFT 0b1
#define MOUSE_RIGHT 0b10
#define MOUSE_MIDDLE 0b100

typedef struct
{
    uint8_t buttons;
    int8_t dx;
    int8_t dy;
} mouse_packet_t;

typedef enum {
    NO_MOUSE = 0,
    MOUSE_EXISTS = 1,
} mouse_existence_t;

typedef enum {
    MOUSE_EVENT_MOUSEUP = 0,
    MOUSE_EVENT_MOUSEDOWN = 1,
    MOUSE_EVENT_MOVE
} mouse_click_event_t;

typedef void (*event_on_click_fn)(int32_t mouse_x, int32_t mouse_y, mouse_click_event_t mouse_edge_type);
typedef void (*event_on_move_fn)(int32_t mouse_x, int32_t mouse_y, int8_t mouse_dx, int8_t mouse_dy);

mouse_existence_t mouse_init();
void mouse_interrupt_handler(regs32_t r);

#endif