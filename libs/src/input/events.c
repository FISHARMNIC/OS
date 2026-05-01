#include <mouse.h>
#include <keyboard.h>
#include <events.h>
#include <sys/string.h>

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

static event_on_click_fn _event_click_functions_bj[CLICK_FUNCTIONS_SIZE];
static event_on_move_fn _event_move_functions_bj[MOVE_FUNCTIONS_SIZE];
static event_on_key_fn _event_keyboard_functions_bj[KB_FUNCTIONS_SIZE];
static uint32_t _event_click_functions_last_bj;
static uint32_t _event_move_functions_last_bj;
static uint32_t _event_keyboard_functions_last_bj;

void events_before_userspace()
{
    // backup handlers
    memcpy((void *)_event_click_functions_bj, (void *)event_click_functions, CLICK_FUNCTIONS_SIZE);
    memcpy((void *)_event_move_functions_bj, (void *)event_move_functions, MOVE_FUNCTIONS_SIZE);
    memcpy((void *)_event_keyboard_functions_bj, (void *)event_keyboard_functions, KB_FUNCTIONS_SIZE);
    _event_click_functions_last_bj = event_click_functions_last;
    _event_move_functions_last_bj = event_move_functions_last;
    _event_keyboard_functions_last_bj = event_keyboard_functions_last;

    // wipe
    memset((void *)event_click_functions, 0, CLICK_FUNCTIONS_SIZE);
    memset((void *)event_move_functions, 0, MOVE_FUNCTIONS_SIZE);
    memset((void *)event_keyboard_functions, 0, KB_FUNCTIONS_SIZE);
    event_click_functions_last = 0;
    event_move_functions_last = 0;
    event_keyboard_functions_last = 0;
}

void events_after_userspace()
{
    // load backups
    memcpy((void *)event_click_functions, (void *)_event_click_functions_bj, CLICK_FUNCTIONS_SIZE);
    memcpy((void *)event_move_functions, (void *)_event_move_functions_bj, MOVE_FUNCTIONS_SIZE);
    memcpy((void *)event_keyboard_functions, (void *)_event_keyboard_functions_bj, KB_FUNCTIONS_SIZE);

    event_click_functions_last = _event_click_functions_last_bj;
    event_move_functions_last = _event_move_functions_last_bj;
    event_keyboard_functions_last = _event_keyboard_functions_last_bj;
}