#include "inc/server.h"
#include "inc/launch.h"
#include "inc/render.h"

#include <userspace/stdio.h>

void server()
{
    render_init_errors_t err = render_init();

    if (err != RIN_OK)
    {
        printf("[LAUNCH FAILED] Code: %d\n", err);
    }
    else
    {
        while (input.keyboard.sc != KEY_ESC)
        {
            render();
        }
        render_deinit();
    }
}