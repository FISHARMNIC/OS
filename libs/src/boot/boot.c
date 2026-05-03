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
#include <os_setjmp.h>
#include <sys/kmalloc.h>

static char* welcome_msg = "\
                                              \n\
 `7MM\"\"\"Mq.  `7MMF'`7MMF'            db       \n\
   MM   `MM.   MM    MM             ;MM:      \n\
   MM   ,M9    MM    MM            ,V^MM.     \n\
   MMmmdM9     MM    MM           ,M  `MM     \n\
   MM  YM.     MM    MM      ,    AbmmmqMA    \n\
   MM   `Mb.   MM    MM     ,M   A'     VML   \n\
 .JMML. .JMM..JMML..JMMmmmmMMM .AMA.   .AMMA. \n\
                                              \n";
                                            
                                            

uint32_t postboot_init(multiboot_info_t* mbi)
{
    paging_init();

    // Setup default framebuffer
    graphics_init_fb(&graphics_fb_default, mbi);

    // Setup default context
    graphics_init_context(&graphics_context_default, &graphics_fb_default, (font_info_t){
        .char_width = 8,
        .char_height = 16,
        .ptr = _binary_FONT_F16_start
    }, COLOR_BLACK, COLOR_WHITE);
    
    graphics_set_active_context(&graphics_context_default);
    
    tty_reset();
    tty_clear();

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

    mem_init();
    mm_init();

    tty_puts("... Kmalloc Ready\n");

    keyboard_init();

    // graphics_context_active->color_bg = COLOR_FORMAT_RGB(60, 50, 60);

    tty_clear();

    graphics_context_active->color_bg=COLOR_FORMAT_RGB(240, 230, 230);
    graphics_context_active->color_fg=COLOR_WHITE;
    
    uint32_t i = 0;
    uint32_t b = 0;
    while(welcome_msg[i])
    {
        graphics_context_active->color_fg = COLOR_FORMAT_RGB((i >> 1) + 50, 50, (i >> 1) + 50);
        graphics_context_active->color_bg = COLOR_FORMAT_RGB(b + 60, 50, b + 60);
        tty_putch(welcome_msg[i]);
        if(welcome_msg[i] == '\n')
        {
            b += 3;
        }
        i++;
    }

    graphics_context_active->color_fg=COLOR_BLACK;
    graphics_context_active->color_bg=COLOR_WHITE;

    return 0;
}
