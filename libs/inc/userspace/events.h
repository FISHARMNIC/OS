#ifndef USER_EVENTS_H
#define USER_EVENTS_H

#include <stdint.h>
#include <events.h>
#include <syscalls.h>

// @todo are these handlers running in ring0? thats not ideal if so

#define HANDLE_INVALID -1
#define HANDLE_NONE HANDLE_INVALID

typedef int32_t handle_t;

typedef struct
{
   handle_t click;
   handle_t move;
} mouse_ev_handles_t;

static inline mouse_ev_handles_t user_events_add_mouse(event_on_click_fn click, event_on_move_fn move)
{
    mouse_ev_handles_t handles; 

    SYSCALL_4PARAM(SYSCALL_HAND_ATTACH_MOUSE, click, move, &handles.click, &handles.move);

    return handles;
}

static inline handle_t user_events_add_keyboard(event_on_key_fn key_event)
{
    handle_t handle;

    SYSCALL_2PARAM(SYSCALL_HAND_ATTACH_KB, key_event, &handle);

    return handle;
}

static inline mouse_ev_handles_t user_events_remove(handle_t click, handle_t move, handle_t key)
{
    SYSCALL_3PARAM(SYSCALL_HAND_REMOVE, click, move, key);
}

#endif