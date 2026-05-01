#include <mouse.h>
#include <keyboard.h>
#include <events.h>

volatile event_on_click_fn event_click_functions[CLICK_FUNCTIONS_SIZE] = {0};
volatile event_on_move_fn event_move_functions[MOVE_FUNCTIONS_SIZE] = {0};
volatile event_on_key_fn event_keyboard_functions[KB_FUNCTIONS_SIZE] = {0};

uint32_t event_click_functions_last = 0;
uint32_t event_move_functions_last = 0;
uint32_t event_keyboard_functions_last = 0;

ADD_HANDLER(click, event_on_click_fn, event_click_functions, event_click_functions_last, CLICK_FUNCTIONS_SIZE)
ADD_HANDLER(move, event_on_move_fn, event_move_functions, event_move_functions_last, MOVE_FUNCTIONS_SIZE)
ADD_HANDLER(keyboard, event_on_key_fn, event_keyboard_functions, event_keyboard_functions_last, KB_FUNCTIONS_SIZE)

REMOVE_HANDLER(click, event_on_click_fn, event_click_functions, event_click_functions_last, CLICK_FUNCTIONS_SIZE)
REMOVE_HANDLER(move, event_on_move_fn, event_move_functions, event_move_functions_last, MOVE_FUNCTIONS_SIZE)
REMOVE_HANDLER(keyboard, event_on_key_fn, event_keyboard_functions, event_keyboard_functions_last, KB_FUNCTIONS_SIZE)