#include <interrupts.h>
#include <ports.h>

void pic_sendEOI(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_irq_disable(uint8_t irq_num)
{
    uint8_t irq_bit;
    uint8_t pic_mask;

    if (irq_num <= 7) {
        pic_mask = inb(PIC1_DATA);
    } else {
        pic_mask = inb(PIC2_DATA);
    }

    irq_bit = (uint8_t)(1U << (irq_num % 8));
    pic_mask = (uint8_t)(irq_bit | pic_mask);

    if (irq_num <= 7) {
        outb(PIC1_DATA, pic_mask);
    } else {
        outb(PIC2_DATA, pic_mask);
    }
}

void pic_disable_irq(uint8_t irq_num)
{
    pic_irq_disable(irq_num);
}

void pic_irq_enable(uint8_t irq_num)
{
    uint8_t irq_bit;
    uint8_t pic_mask;

    if (irq_num <= 7) {
        pic_mask = inb(PIC1_DATA);
    } else {
        pic_mask = inb(PIC2_DATA);
    }

    irq_bit = (uint8_t)~(1U << (irq_num % 8));
    pic_mask = (uint8_t)(irq_bit & pic_mask);

    if (irq_num <= 7) {
        outb(PIC1_DATA, pic_mask);
    } else {
        outb(PIC2_DATA, pic_mask);
    }
}

void pic_enable_irq(uint8_t irq_num)
{
    pic_irq_enable(irq_num);
}

void pic_remap(void)
{
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, 0x1);
    outb(PIC2_DATA, 0x1);

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}
