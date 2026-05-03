.set MAGIC, 0x1BADB002
.set MBOOT_PAGE_ALIGN, 1 << 0
.set MBOOT_MEM_INFO, 1 << 1
.set MBOOT_GRAPH_MODE, 1 << 2
.set FLAGS, MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_GRAPH_MODE
.set CHECKSUM, -(MAGIC + FLAGS)

.set CODE_SEG, 0x08
.set DATA_SEG, 0x10

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 640
.long 480
.long 32
// .long 1280
// .long 720
// .long 32

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .data
.align 8
.global isr_exception_type
isr_exception_type: .byte 0

.global gdt_start, gdt_code, _CODE_SEG, _DATA_SEG, _USER_CODE_SEG, _USER_DATA_SEG, gdt_tss, stack_top
gdt_start:
    .long 0x00000000
    .long 0x00000000
gdt_code:
    .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
gdt_data:
    .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
gdt_user_code:
    .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFA, 0xCF, 0x00
gdt_user_data:
    .byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00
gdt_tss:
    .word 0x0067        // limit = sizeof(tss)-1
    .word 0x0000        // base bits 0-15 — fill at runtime
    .byte 0x00          // base 16-23
    .byte 0x89          // access: present, DPL=0, TSS32 available
    .byte 0x00          // granularity
    .byte 0x00          // base 24-31 
gdt_end:
gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start
_CODE_SEG:
    .long gdt_code - gdt_start
_DATA_SEG:
    .long gdt_data - gdt_start
_USER_CODE_SEG:
    .long gdt_user_code - gdt_start
_USER_DATA_SEG:
    .long gdt_user_data - gdt_start

.section .text
.global _start
.type _start, @function

.extern kernel_entry
.extern interrupts_exception_handler
.extern interrupts_irq_handler

.macro isr_err_stub n
isr_stub_\n:
    movb $\n, isr_exception_type
    cli
    pushal
    call interrupts_exception_handler
    popal
    addl $4, %esp
    iret
.endm

.macro isr_no_err_stub n
isr_stub_\n:
    movb $\n, isr_exception_type
    cli
    pushal
    call interrupts_exception_handler
    popal
    iret
.endm

.macro irq_stub n
isr_stub_\n:
    movb $\n, isr_exception_type
    cli
    pushal
    call interrupts_irq_handler
    popal
    iret
.endm

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_stub 32
irq_stub 33
irq_stub 34
irq_stub 35
irq_stub 36
irq_stub 37
irq_stub 38
irq_stub 39
irq_stub 40
irq_stub 41
irq_stub 42
irq_stub 43
irq_stub 44
irq_stub 45
irq_stub 46
irq_stub 47
irq_stub 48
irq_stub 49
irq_stub 50

.global isr_stub_table
isr_stub_table:
    .long isr_stub_0
    .long isr_stub_1
    .long isr_stub_2
    .long isr_stub_3
    .long isr_stub_4
    .long isr_stub_5
    .long isr_stub_6
    .long isr_stub_7
    .long isr_stub_8
    .long isr_stub_9
    .long isr_stub_10
    .long isr_stub_11
    .long isr_stub_12
    .long isr_stub_13
    .long isr_stub_14
    .long isr_stub_15
    .long isr_stub_16
    .long isr_stub_17
    .long isr_stub_18
    .long isr_stub_19
    .long isr_stub_20
    .long isr_stub_21
    .long isr_stub_22
    .long isr_stub_23
    .long isr_stub_24
    .long isr_stub_25
    .long isr_stub_26
    .long isr_stub_27
    .long isr_stub_28
    .long isr_stub_29
    .long isr_stub_30
    .long isr_stub_31
    .long isr_stub_32
    .long isr_stub_33
    .long isr_stub_34
    .long isr_stub_35
    .long isr_stub_36
    .long isr_stub_37
    .long isr_stub_38
    .long isr_stub_39
    .long isr_stub_40
    .long isr_stub_41
    .long isr_stub_42
    .long isr_stub_43
    .long isr_stub_44
    .long isr_stub_45
    .long isr_stub_46
    .long isr_stub_47
    .long isr_stub_48
    .long isr_stub_49
    .long isr_stub_50

.global syscall_stub
syscall_stub:
    cli
    pushal
    call syscall_dispatch
    popal
    iret

_start:
    lgdt gdt_descriptor
    movw $DATA_SEG, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $CODE_SEG, $gdt_loaded

gdt_loaded:
    movl $stack_top, %esp
    pushl %ebx
    call kernel_entry
    addl $4, %esp

halt:
    cli
    hlt
    jmp halt

.size _start, . - _start
