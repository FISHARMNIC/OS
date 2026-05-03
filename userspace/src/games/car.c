#include <userspace/vbuff.h>
#include <userspace/events.h>
#include <userspace/malloc.h>

#define GREEN COLOR_FORMAT_RGB(50, 200, 50)
#define DGRAY COLOR_FORMAT_RGB(50, 50, 50)
#define LGRAY COLOR_FORMAT_RGB(100, 100, 100)
#define YELLOW COLOR_FORMAT_RGB(220, 150, 0)

#define CARCOL COLOR_FORMAT_RGB(200, 50, 50)

#define MWID 6
#define BHGHT 50

static framebuffer_t fb;
static framebuffer_t rfb;
static uint8_t *render_buffer = NULLPTR;

static uint32_t width;
static uint32_t height;

static uint32_t width_h;
static uint32_t width_f;
static uint32_t width_s;
static uint32_t width_e;

static uint32_t speed = 0;

static uint32_t carx = 0;
static int32_t lane = 0;

static volatile bool exit = false;

void kb_handler(uint8_t c, keyboard_event_t event)
{
    if (c == KEY_ESC && event == KEYBOARD_EVENT_KEY_RELEASE)
    {
        exit = true;
    }
    else if (event == KEYBOARD_EVENT_KEY_PRESS)
    {
        if (c == KEY_UP && speed < 10)
        {
            speed++;
        }
        else if (c == KEY_DOWN && speed > 0)
        {
            speed--;
        }
        else if (c == KEY_LEFT && lane > -3)
        {
            carx -= width_e;
            lane--;
        }
        else if (c == KEY_RIGHT && lane < 3)
        {
            carx += width_e;
            lane++;
        }
    }
}

void draw_back()
{
    clear_screen(GREEN, &rfb);
    fill_rect(width_f, 0, width_h, height, DGRAY, &rfb);
    fill_rect(width_f - 20, 0, 20, height, LGRAY, &rfb);
    fill_rect(width_f + width_h, 0, 20, height, LGRAY, &rfb);
}

void draw_lines(uint32_t voff)
{
    fill_rect(width_h - width_s - MWID, 0, MWID, height, YELLOW, &rfb);
    fill_rect(width_h - (MWID >> 1), 0, MWID, height, YELLOW, &rfb);
    fill_rect(width_h + width_s, 0, MWID, height, YELLOW, &rfb);

    for (int32_t i = voff - BHGHT; i < (int32_t)height; i += BHGHT)
    {
        fill_rect(width_h - width_s - MWID, i, (width_s << 1) + (MWID << 1), 20, DGRAY, &rfb);
    }
}

void draw_car()
{
    fill_rect(carx, height - width_e - 50, width_e, width_e + 30, CARCOL, &rfb);
    // fill_rect(carx - 5, height - width_e - 50 + 5, 5, 15, COLOR_BLACK, &rfb);
    // fill_rect(carx - 5, height + width_e + 30 - 5, 5, 15, COLOR_BLACK, &rfb);
}

static void swap_buffers()
{
    uint32_t *front = (uint32_t *)fb.addr;
    uint32_t *back = (uint32_t *)rfb.addr;
    uint32_t count = (fb.pitch * fb.height) >> 2;

    for (uint32_t i = 0; i < count; i++)
        front[i] = back[i];
}

int main()
{
    vbuff(&fb);

    width = fb.width;
    height = fb.height;

    uint32_t render_buffer_size = fb.pitch * fb.height;
    render_buffer = malloc(render_buffer_size);
    if (render_buffer == NULLPTR)
    {
        vbuff_dispose(&fb);
        return 1;
    }

    rfb = fb;
    rfb.addr = (uint32_t)render_buffer;

    width_h = width >> 1;
    width_f = width >> 2;
    width_s = width >> 3;
    width_e = width >> 4;

    handle_t kb_handle = user_events_add_keyboard(kb_handler);

    carx = width_h - (width_e >> 1);

    uint8_t i = 0;
    while (!exit)
    {
        draw_back();
        draw_lines(i);
        draw_car();

        i += speed;
        if (i > BHGHT)
        {
            i = 0;
        }

        swap_buffers();
    }

    user_events_remove(HANDLE_NONE, HANDLE_NONE, kb_handle);

    free(render_buffer);
    vbuff_dispose(&fb);

    return 0;
}