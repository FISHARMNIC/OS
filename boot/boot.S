.set ALIGN, 1 << 0
.set MEMINFO, 1 << 1
.set GRAPHICS, 1 << 2
.set FLAGS, ALIGN | MEMINFO | GRAPHICS
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

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

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
.extern kernel_entry

_start:
    movl $stack_top, %esp
    // push %ebx
    push %ebx
    call kernel_entry
    addl $8, %esp

halt:
    cli
    hlt
    jmp halt
