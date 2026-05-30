#include <context.h>
#include <sys/kmalloc.h>
#include <tss.h>
#include <paging.h>
#include <elf.h>

static task_ll_t* task_list_loop = NULLPTR; // loopback
static task_t* current_task = NULLPTR;

// round robin for now with circular dll

task_ll_t* task_add(task_t task)
{
    task_ll_t* new = kmalloc(sizeof(task_ll_t));
    
    new->task = task;
    new->next = NULLPTR;
    new->prev = NULLPTR;

    if(task_list_loop == NULLPTR)
    {
        new->next = new; // create loopback
        new->prev = new;

        task_list_loop = new;
    }
    else
    {
        new->next = task_list_loop->next; // I loopback
        task_list_loop->next = new;       // end is me

        new->prev = task_list_loop;       // I go after old head
        task_list_loop->next->prev = new; // start goes back to me

        task_list_loop = new;             // loopback is me
    }

    return new;
}

void task_remove(task_ll_t* task)
{
    task_ll_t* prev = task->prev;
    task_ll_t* next = task->next;
    
    // slice me out
    prev->next = task->next;
    next->prev = task->prev;

    if(task_list_loop == task) // I am loopback, shift loopback forward
    {
        task_list_loop = task->next;
    }

    if(task->next == task) // I was the only task
    {
        task_list_loop = NULLPTR;
    }

    kfree(task);
}

task_t* task_next()
{
    task_ll_t* tll = task_list_loop;

    if(tll == NULLPTR)
    {
        return NULLPTR;
    }
    task_list_loop = tll->next;
    return &tll->task;
}

task_t* task_get_current(void)
{
    return current_task;
}

void task_set_current(task_t* task)
{
    current_task = task;
}