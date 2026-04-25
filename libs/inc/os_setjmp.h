#ifndef SETJMP_H
#define SETJMP_H

// Source - https://stackoverflow.com/a/46096538
// Posted by Jester, modified by community. See post 'Timeline' for change history
// Retrieved 2026-04-24, License - CC BY-SA 4.0

typedef unsigned long jmp_buf[6];
typedef void (*iret_return_fn_t)(void);

__attribute__((noinline, noclone, returns_twice, optimize(0))) int setjmp(jmp_buf var);
__attribute__((noinline, noclone, optimize(0))) void longjmp(jmp_buf var,int m);

extern jmp_buf kernel_return_ctx;


#endif