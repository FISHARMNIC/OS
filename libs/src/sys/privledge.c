#include <privledge.h>

extern uint32_t _USER_CODE_SEG;
extern uint32_t _USER_DATA_SEG;

void privledge_enter_user_mode(user_entry_fn_t entry, uint32_t user_stack_top)
{
    uint32_t user_cs = (_USER_CODE_SEG & 0xFFFFU) | 0x3U;
    uint32_t user_ds = (_USER_DATA_SEG & 0xFFFFU) | 0x3U;
    uint16_t user_ds16 = (uint16_t)user_ds;

    __asm__ volatile(
        "cli\n\t"
        "movw %[ds], %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "pushl %[dspush]\n\t"
        "pushl %[ustack]\n\t"
        "pushfl\n\t"
        "orl $0x200, (%%esp)\n\t"
        "pushl %[cs]\n\t"
        "pushl %[entry]\n\t"
        "iret\n\t"
        :
        : [ds] "r"(user_ds16),
          [dspush] "r"(user_ds),
          [cs] "r"(user_cs),
          [ustack] "r"(user_stack_top),
          [entry] "r"(entry)
        : "eax", "memory");

    __builtin_unreachable();
}
