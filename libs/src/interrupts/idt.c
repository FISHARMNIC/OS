#include <interrupts.h>

uint8_t _interrupts_enabled_ = 1;

static __attribute__((aligned(0x10))) idt_entry_t idt[IDT_MAX_DESCRIPTORS];
static interrupt_fn_t idt_customs[IRQ_LINE_COUNT];
static idtr_t idtr;

void idt_load_interrupt(uint8_t irq, interrupt_fn_t isr)
{
    if (irq < IRQ_LINE_COUNT) {
        idt_customs[irq] = isr;
    }
}

void idt_set_gate(uint8_t vector, void *isr, uint8_t flags)
{
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low = (uint32_t)isr & 0xFFFFU;
    descriptor->kernel_cs = (uint16_t)_CODE_SEG;
    descriptor->reserved = 0;
    descriptor->attributes = flags;
    descriptor->isr_high = ((uint32_t)isr >> 16) & 0xFFFFU;
}

void idt_load_stubs(void)
{
    uint8_t vector = 0;

    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1U);

    for (; vector < 32; ++vector) {
        idt_set_gate(vector, isr_stub_table[vector], 0x8E);
    }

    for (vector = 32; vector < IDT_CURRENT_ENTRIES; ++vector) {
        idt_set_gate(vector, isr_stub_table[vector], 0x8E);
    }

    for (vector = 0; vector < IRQ_LINE_COUNT; ++vector) {
        idt_customs[vector] = 0;
    }

    __asm__ volatile("lidt %0" : : "m"(idtr));
}

void interrupts_enable(void)
{
    __asm__ volatile("sti");
}

void interrupts_disable(void)
{
    __asm__ volatile("cli");
}

void interrupts_exception_handler(void)
{
    regs32_t regs = {0};

    _interrupts_enabled_ = 0;
    (void)regs;
    interrupts_disable();
    __asm__ volatile("hlt");
}

void interrupts_irq_handler(void)
{
    uint8_t vector = (uint8_t)isr_exception_type;
    uint8_t irq;
    regs32_t regs = {0};

    _interrupts_enabled_ = 0;

    if (vector < OFFSET_PIC1 || vector >= OFFSET_PIC2 + IRQ_LINE_COUNT / 2) {
        _interrupts_enabled_ = 1;
        return;
    }

    irq = (uint8_t)(vector - OFFSET_PIC1);

    if (irq < IRQ_LINE_COUNT && idt_customs[irq] != 0) {
        idt_customs[irq](regs);
    }

    pic_sendEOI(irq);
    _interrupts_enabled_ = 1;
}
