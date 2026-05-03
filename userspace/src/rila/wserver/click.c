#include "inc/click.h"
#include <userspace/malloc.h>

static click_ll_t* base = NULLPTR;
static click_ll_t* head = NULLPTR;

void add_click(click_ll_fn_t event)
{
    click_ll_t* new = malloc(sizeof(click_ll_t));
    new->fn = event;

    if(head == NULLPTR)
    {
        head = new;

        new->prev = NULLPTR;
        new->next = NULLPTR;

        base = head;
    }
    else
    {
        head->next = new;

        new->prev = head;
        new->next = NULLPTR;

        head = new;
    }
}

void remove_click(click_ll_fn_t event)
{
    click_ll_t* walker = base;

    while(walker != NULLPTR)
    {
        click_ll_t* current = walker;

        if(current->fn == event)
        {
            if(current->prev != NULLPTR)
            {
                click_ll_t* prev = current->prev;
                prev->next = current->next;
            }
            if(current->next != NULLPTR)
            {
                click_ll_t* next = current->next;
                next->prev = current->prev;
            }

            if(head == current)
            {
                head = current->prev;
            }
            if(base == current)
            {
                base = current->next;
            }

            free(current);
            return;
        }
        walker = walker->next;
    }
}

void handle_click(int32_t x, int32_t y, mouse_click_event_t e)
{
    click_ll_t* walker = head;

    while(walker != NULLPTR)
    {
        bool hit = walker->fn(x, y, e);
        if(hit)
        {
            return;
        }
        walker = walker->prev;
    }
}