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


typedef void event_on_click_fn(int32_t, int32_t, int8_t);
typedef void event_on_move_fn(int32_t, int32_t);

extern event_on_click_fn *mouse_on_click_fn;
extern event_on_move_fn *mouse_on_move_fn;

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

mouse_existence_t mouse_init();
void mouse_interrupt_handler(regs32_t r);

#endif