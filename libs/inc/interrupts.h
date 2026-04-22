#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <cpu.h>
#include <ports.h>

#define IDT_MAX_DESCRIPTORS 256
#define IDT_CURRENT_ENTRIES 51
#define IRQ_LINE_COUNT 16

typedef void (*interrupt_fn_t)(regs32_t);

typedef struct __attribute__((packed)) {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idtr_t;

extern int8_t isr_exception_type;
extern uint32_t _CODE_SEG;
extern uint32_t *isr_stub_table[];

enum {
    IRQ_TIMER,
    IRQ_KEYBOARD,
    IRQ_CASCADE,
    IRQ_COM2,
    IRQ_COM1,
    IRQ_LPT2,
    IRQ_FLOPPY,
    IRQ_LPT1,
    IRQ_CMOS,
    IRQ_PERIPH1,
    IRQ_PERIPH2,
    IRQ_PERIPH3,
    IRQ_MOUSE,
    IRQ_COP,
    IRQ_ATA_PRIM,
    IRQ_ATA_SEC,
};

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20
#define OFFSET_PIC1 32
#define OFFSET_PIC2 40

void idt_load_stubs(void);
void idt_load_interrupt(uint8_t irq, interrupt_fn_t isr);
void idt_set_gate(uint8_t vector, void *isr, uint8_t flags);

void interrupts_enable(void);
void interrupts_disable(void);

void pic_sendEOI(uint8_t irq);
void pic_irq_disable(uint8_t irq_num);
void pic_irq_enable(uint8_t irq_num);
void pic_enable_irq(uint8_t irq_num);
void pic_disable_irq(uint8_t irq_num);
void pic_remap(void);

void interrupts_exception_handler(void);
void interrupts_irq_handler(void);

#endif
