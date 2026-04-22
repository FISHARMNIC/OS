#include <graphics.h>
#include <ports.h>
#include <mouse.h>
#include <cpu.h>

static char mouse_cycle = 0;
static mouse_packet_t mouse_byte;
static int32_t mouse_x = 0;
static int32_t mouse_y = 0;
volatile uint8_t mouse_down = 0;
static uint16_t saveBuffer[CHAR_HEIGHT * CHAR_WIDTH];

static volatile int8_t mouse_edgeType = 0;

event_on_click_fn *mouse_on_click_fn = NULLPTR;
event_on_move_fn *mouse_on_move_fn = NULLPTR;

static void mouse_render()
{
    graphics_draw_char('X', mouse_x, mouse_y, &graphics_context_default);
}

static void mouse_handle_click(uint8_t data)
{
    mouse_down = data & MOUSE_BUTTONS_MASK;

    if ((mouse_on_click_fn != NULLPTR) &&
        ((mouse_edgeType == 0 && mouse_down > 0) || (mouse_edgeType == 1 && mouse_down == 0))) // clicked or falling action
    {
        mouse_edgeType = mouse_down > 0;
        mouse_on_click_fn(mouse_x, mouse_y, mouse_edgeType);
    }
}

static void mouse_handle_move(int8_t dx, int8_t dy)
{
    mouse_x += dx;
    mouse_y -= dy;

    if (mouse_x < 0) {
        mouse_x = 0;
    } else if (mouse_x >= (int32_t)(graphics_fb_active->width - CHAR_WIDTH)) {
        mouse_x = graphics_fb_active->width - CHAR_WIDTH - 1;
    }

    if (mouse_y < 0) {
        mouse_y = 0;
    } else if (mouse_y >= (int32_t)(graphics_fb_active->height - CHAR_HEIGHT)) {
        mouse_y = graphics_fb_active->height - CHAR_HEIGHT - 1;
    }

    if (mouse_on_move_fn != NULLPTR)
    {
        mouse_on_move_fn(mouse_x, mouse_y);
    }

    mouse_render();
}

void mouse_interrupt_handler(regs32_t r)
{
    (void)r;

    switch (mouse_cycle)
    {
    case 0:
        mouse_byte.buttons = inb(MOUSE_PORT_1);
        mouse_handle_click(mouse_byte.buttons);
        mouse_cycle++;
        break;
    case 1:
        mouse_byte.dx = inb(MOUSE_PORT_1);
        mouse_cycle++;
        break;
    case 2:
        mouse_byte.dy = inb(MOUSE_PORT_1);
        mouse_handle_move(mouse_byte.dx, mouse_byte.dy);
        mouse_cycle = 0;
        break;
    }
}

static void mouse_wait(uint8_t a_type) 
{
    uint32_t _time_out = 1000000;
    
    if (a_type == 0)
    {
        while (_time_out--) // Data
        {
            if ((inb(MOUSE_PORT_2) & 1) == 1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while (_time_out--) // Signal
        {
            if ((inb(MOUSE_PORT_2) & 2) == 0)
            {
                return;
            }
        }
        return;
    }
}

static void mouse_write(uint8_t a_write)
{
    mouse_wait(1);
    outb(MOUSE_PORT_2, MOUSE_PACKET_HEADER);
    mouse_wait(1);
    outb(MOUSE_PORT_1, a_write);
}

mouse_existence_t mouse_detect()
{
    mouse_wait(0);
    uint8_t tmp = inb(MOUSE_PORT_1);

    if (tmp != MOUSE_ACK)
        return NO_MOUSE;
    else
        return MOUSE_EXISTS;
}

mouse_existence_t mouse_init()
{
    if(mouse_detect() == NO_MOUSE)
    {
        return NO_MOUSE;
    }

    mouse_write(MOUSE_ENABLE_PACKET_STREAMING);

    while (inb(MOUSE_PORT_1) != MOUSE_ACK);

    outb(MOUSE_PORT_2, MOUSE_ENABLE_INT);

    uint8_t res = inb(MOUSE_PORT_1);
    res |= 2;

    outb(MOUSE_PORT_2, MOUSE_PORT_1);
    outb(MOUSE_PORT_1, res);

    for (uint32_t i = 0; i < CHAR_HEIGHT * CHAR_WIDTH; ++i) {
        saveBuffer[i] = 0;
    }

    return MOUSE_EXISTS;
}
