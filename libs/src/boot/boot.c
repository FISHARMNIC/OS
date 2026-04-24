#include <graphics.h>
#include <boot.h>
#include <interrupts.h>
#include <mouse.h>
#include <paging.h>
#include <keyboard.h>
#include <fat.h>
#include <disk.h>
#include <syscalls.h>
#include <tss.h>

uint32_t postboot_init(multiboot_info_t* mbi)
{
    paging_init();

    // Setup default framebuffer
    graphics_init_fb(&graphics_fb_default, mbi);
    graphics_fb_active = &graphics_fb_default;

    // Setup default context
    graphics_init_context(&graphics_context_default, &graphics_fb_default, _binary_FONT_F16_start, COLOR_WHITE, COLOR_BLACK);
    
    tty_reset();
    tty_puts("... Paging and Graphics enabled\n");

    // Load interrupts into IDT and register custom handlers.
    idt_load_stubs();
    idt_load_interrupt(IRQ_MOUSE, mouse_interrupt_handler);
    idt_load_interrupt(IRQ_KEYBOARD, keyboard_handler);

    tty_puts("... IDT initialized\n");

    tss_init();

    tty_puts("... TSS initialized\n");

    // Move IRQs to vectors 32-47 and unmask needed lines.
    pic_remap();
    pic_enable_irq(IRQ_CASCADE); // Slave PIC
    pic_enable_irq(IRQ_MOUSE);
    pic_enable_irq(IRQ_KEYBOARD);

    tty_puts("... PIC enabled\n");

    // Enable mouse streaming
    mouse_init();
    keyboard_init();

    tty_puts("... Peripherals enabled\n");
    
    syscalls_init();  
    
    tty_puts("... Syscalled setup\n");

    interrupts_enable();
  
    tty_puts("... Interrupts enabled\n");

    uint32_t resp = ata_send_identify(NULLPTR); 
    if(resp)
    {
        return 1;
    }

    tty_puts("... Disk Ready\n... Setting up FAT\n");

    resp = fat32_init();
    if (resp)
    {
        return 1;
    }

    tty_puts("... FAT Ready\n");

    return 0;
}
