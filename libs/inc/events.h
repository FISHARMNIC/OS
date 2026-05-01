#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

#include <mouse.h>
#include <keyboard.h>
#include <graphics.h>

#define CLICK_FUNCTIONS_SIZE 8
#define MOVE_FUNCTIONS_SIZE 8
#define KB_FUNCTIONS_SIZE 8

extern volatile event_on_click_fn event_click_functions[CLICK_FUNCTIONS_SIZE];
extern volatile event_on_move_fn event_move_functions[MOVE_FUNCTIONS_SIZE];
extern volatile event_on_key_fn event_keyboard_functions[KB_FUNCTIONS_SIZE];

extern uint32_t event_click_functions_last;
extern uint32_t event_move_functions_last;
extern uint32_t event_keyboard_functions_last;

#define ADD_HANDLER_PROTO(name, type) int32_t event_##name##_add_handler(type fn)
#define REMOVE_HANDLER_PROTO(name, type) int32_t event_##name##_remove_handler(int32_t index)

// @todo maybe rework to remove by pointer? that way its not fragged

#define ADD_HANDLER(name, type, arr, last, size) \
    ADD_HANDLER_PROTO(name, type)                \
    {                                            \
        if (fn == NULLPTR)                       \
        {                                        \
            return -1;                           \
        }                                        \
        for (uint32_t i = 0; i < size; i++)      \
        {                                        \
            if (arr[i] == NULLPTR)               \
            {                                    \
                arr[i] = fn;                     \
                if (i > last)                    \
                {                                \
                    last = i;                    \
                }                                \
                return i;                        \
            }                                    \
        }                                        \
        return -1;                               \
    }

#define REMOVE_HANDLER(name, type, arr, last, size)   \
    REMOVE_HANDLER_PROTO(name, type)                  \
    {                                                 \
                                                      \
        if ((uint32_t)index >= size || index < 0)     \
        {                                             \
            /*tty_puts("NOT REMOVING " #name "\n");*/ \
            return -1;                                \
        }                                             \
        /*tty_puts("REMOVING " #name "\n");*/         \
        if ((uint32_t)index == last && index != 0)    \
        {                                             \
            last--;                                   \
        }                                             \
        arr[index] = NULLPTR;                         \
        return 0;                                     \
    }

#define FOREACH(arr, last, ...)          \
    for (uint32_t i = 0; i <= last; i++) \
    {                                    \
        if (arr[i] != NULLPTR)           \
        {                                \
            arr[i](__VA_ARGS__);         \
        }                                \
    }

ADD_HANDLER_PROTO(click, event_on_click_fn);
ADD_HANDLER_PROTO(move, event_on_move_fn);
ADD_HANDLER_PROTO(keyboard, event_on_key_fn);

REMOVE_HANDLER_PROTO(click, event_on_click_fn);
REMOVE_HANDLER_PROTO(move, event_on_move_fn);
REMOVE_HANDLER_PROTO(keyboard, event_on_key_fn);

void events_before_userspace();
void events_after_userspace();
#endif