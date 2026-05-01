#include <userspace/events.h>
#include <userspace/stdio.h>
#include <stdbool.h>

volatile bool exit = false;

void kb_handler(uint8_t c)
{
    if(c == KEY_ENTER)
    {
        exit = true;
    }
    else
    {
        printf("\tKeycode: %d\n", c);
    }
}

int main(int argc, char* argv[])
{
    handle_t handle = user_events_add_keyboard(kb_handler);

    printf("Events attached... Hit [ENTER] to exit.\n");

    while(!exit);

    printf("\tExiting\n");

    user_events_remove(HANDLE_NONE, HANDLE_NONE, handle);

    return 0;
}