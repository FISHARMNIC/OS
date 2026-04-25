#include <tss.h>
#include <sys/string.h>
#include <graphics.h>

static tss_t tss;

void tss_set_esp0(uint32_t esp)
{
    tss.esp0 = esp;
}

void tss_init()
{
    uint32_t base = (uint32_t)&tss;

    gdt_tss[2] = base & 0xFF;
    gdt_tss[3] = (base >> 8) & 0xFF;
    gdt_tss[4] = (base >> 16) & 0xFF;
    gdt_tss[7] = (base >> 24) & 0xFF;

    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;       // kernel data selector
    tss.esp0 = (uint32_t)&stack_top; // top of kernel stack
    tss.iomap_base = sizeof(tss);

    tty_printf("\t[TSS] esp0 = %d\n", tss.esp0);

    // @todo 0x28 is because of entry num in gdt, make dynamic
    __asm__ volatile("ltr %%ax" : : "a"((uint16_t)0x28));
}