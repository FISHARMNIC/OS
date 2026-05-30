#ifndef CONTEXT_H
#define CONTEXT_H

#include <cpu.h>

typedef enum
{
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_ZOMBIE
} task_state_t;

typedef struct
{
    uint32_t pid;
    task_state_t state;
    regs32_t regs;
    uint32_t *page_directory;
    uint32_t kernel_stack;
} task_t;

typedef struct task_ll
{
    task_t task;
    struct task_ll *prev;
    struct task_ll *next;
} task_ll_t;

task_ll_t* task_add(task_t task);
void task_remove(task_ll_t* task);
task_t* task_next();

task_t* task_get_current(void);
void task_set_current(task_t* task);
// void task_switch(regs32_t regs);

extern void task_restore_registers(regs32_t* regs);

#endif