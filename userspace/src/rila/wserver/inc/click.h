#ifndef CLICK_H
#define CLICK_H

#include <stdbool.h>
#include <stdint.h>

#include <userspace/events.h>

typedef bool (*click_ll_fn_t) (int32_t x, int32_t y, mouse_click_event_t e);

typedef struct click_ll
{
    click_ll_fn_t fn;
    struct click_ll* prev;
    struct click_ll* next;
} click_ll_t;

void add_click(click_ll_fn_t event);
void remove_click(click_ll_fn_t event);
void handle_click(int32_t x, int32_t y, mouse_click_event_t e);

#endif