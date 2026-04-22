#include <keyboard.h>
#include <cpu.h>
#include <ports.h>

static volatile uint8_t keyboard_sc = 0; // can't cache in keyboard_getch
volatile event_on_key_fn keyboard_on_press_fn = NULLPTR;
static const char KEYSET[128] = "`^1234567890-=\b\tqwertyuiop[]\\ asdfghjkl;'\n  zxcvbnm,./      ~!@#$%^&*()_+  QWERTYUIOP{}| ASDFGHJKL:\"   ZXCVBNM<>?";

void keyboard_init()
{
    // keyboard_get_keycode(); // load out first char
}

void keyboard_handler(regs32_t r)
{
    keyboard_sc = inb(KEYBOARD_PORT);
    if ((keyboard_sc < KEYCODE_RISING) && (keyboard_on_press_fn != NULLPTR))
    {
        (keyboard_on_press_fn)(keyboard_sc);
    }
}

uint8_t keyboard_get_keycode()
{
    while (keyboard_sc >= KEYCODE_RISING)
        ; // wait until press
    uint8_t old = keyboard_sc;
    while (keyboard_sc == old)
        ;                                                      // wait until release or other key pressed
    return old >= KEYCODE_RISING ? old - KEYCODE_RISING : old; // get original char
}

char keyboard_keycode_to_char(uint8_t keycode)
{
    if (keycode >= sizeof(KEYSET)) {
        return 0;
    }
    return KEYSET[keycode];
}

char keyboard_getch()
{
    // event_on_key_fn onk = keyboard_on_press_fn;
    // keyboard_on_press_fn = NULLPTR; // destroy event handler to prioritize stdio

    char ret = keyboard_get_keycode();

    // keyboard_on_press_fn = onk;
    return keyboard_keycode_to_char(ret);
}

void keyboard_gets(char *buffer, uint32_t len)
{
    // event_on_key_fn onk = keyboard_on_press_fn; // destroy event handler to prioritize stdio
    // keyboard_on_press_fn = NULLPTR;

    uint32_t index = 0;

    char got = 0;

    while ((got = keyboard_get_keycode()) != KEY_ENTER)
    {
        if (got == KEY_BACKSPACE && index > 0)
        {
            index--;
            buffer[index] = 0;
        }
        else if (got == KEY_ESC)
        {
            break;
        }
        else if (got == KEY_TAB)
        {
            buffer[index] = ' ';
            index++;
        }
        else
        {
            buffer[index] = keyboard_keycode_to_char(got);
            index++;
        }

        if (index >= len - 1)
        {
            break;
        }
    }

    buffer[index] = 0;

    // keyboard_on_press_fn = onk;
    return;
}
