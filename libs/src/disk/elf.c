#include <paging.h>
#include <elf.h>
#include <sys/string.h>
#include <graphics.h>
#include <cpu.h>
#include <tss.h>
#include <interrupts.h>

static elf_error_t elf_validate(const elf_header_t *hdr)
{
    if (hdr->ident[EI_MAG0] != ELFMAG0 ||
        hdr->ident[EI_MAG1] != ELFMAG1 ||
        hdr->ident[EI_MAG2] != ELFMAG2 ||
        hdr->ident[EI_MAG3] != ELFMAG3)
        return ELF_ERR_MAGIC;

    if (hdr->ident[EI_CLASS] != ELFCLASS32)
        return ELF_ERR_CLASS;

    if (hdr->type != ET_EXEC)
        return ELF_ERR_TYPE;

    if (hdr->machine != EM_386)
        return ELF_ERR_MACHINE;

    if (hdr->phnum == 0)
        return ELF_ERR_NOPHDR;

    return ELF_OK;
}

/*
 * Get a pointer to the Nth program header.
 * phoff is the byte offset from the start of the file to the phdr table.
 * phentsize is the size of each entry in the phdr table.
 */
static const elf_ph_t *elf_get_ph(const uint8_t *file, const elf_header_t *hdr, uint16_t idx)
{
    return (const elf_ph_t *)(file + hdr->phoff + (idx * hdr->phentsize));
}

/**
 * @brief Parses the ELF file at @p file, loads all PT_LOAD segments into memory at their requested virtual addresses (identity-mapped, so vaddr == paddr),
 * marks the covered pages as user-accessible, and fills in "out" with info about where the .text and .data segments are within the file.
 * On success, out->entry_fileOff holds the virtual entry point (e_entry).
 */
elf_error_t elf_load(const uint8_t *file_bytes, uint32_t file_size, elf_section_offsets_t *out)
{
    tty_printf("[ELF] Loading file at %d, size %d\n", (uint32_t)file_bytes, file_size);

    const elf_header_t *hdr = (const elf_header_t *)file_bytes;
    elf_error_t err = elf_validate(hdr);
    if (err != ELF_OK)
    {
        tty_puts("[ELF] Error! Invalid file\n");
        return err;
    }

    out->file = (uint8_t *)file_bytes;
    out->file_size = file_size;
    out->entry_fileOff = hdr->entry;
    out->text_offset = 0;
    out->text_size = 0;
    out->data_offset = 0;
    out->data_size = 0;

    for (uint16_t i = 0; i < hdr->phnum; i++)
    {
        // tty_printf("[ELF] Loading program header %d : %d\n", i, hdr->phnum);

        const elf_ph_t *ph = elf_get_ph(file_bytes, hdr, i);

        if (ph->type != PT_LOAD)
            continue;

        if (ph->memsz == 0)
            continue;

        // tty_printf("\t[ELF] Moving offset into virt addr\n");

        // File into virt addr
        memcpy((void *)ph->vaddr, file_bytes + ph->offset, ph->filesz);

        // Zero BSS
        if (ph->memsz > ph->filesz)
        {
            memset((void *)(ph->vaddr + ph->filesz), 0, ph->memsz - ph->filesz);
        }

        // tty_printf("\t[ELF] Giving user permissions [%d - %d]\n", ph->vaddr, ph->vaddr + ph->memsz);

        // Set user
        paging_set_user_range(ph->vaddr, ph->memsz);

        if (ph->flags & PF_X) // Executable (.text)
        {
            out->text_offset = ph->offset;
            out->text_size = ph->filesz;
        }
        else // Non-executable (.data, .rodata, .bss)
        {
            out->data_offset = ph->offset;
            out->data_size = ph->memsz; // memsz covers BSS too
        }

        // tty_printf("[ELF] ph->vaddr   = %d\n", ph->vaddr);
        // tty_printf("[ELF] ph->offset  = %d\n", ph->offset);
        // tty_printf("[ELF] ph->filesz  = %d\n", ph->filesz);
        // tty_printf("[ELF] copying from file+%d to %d\n", ph->offset, ph->vaddr);

        // uint8_t *dst = (uint8_t *)ph->vaddr;
        // tty_printf("[ELF] bytes at entry: %d %d %d %d\n", dst[0], dst[1], dst[2], dst[3]);
    }

    // tty_puts("[ELF] Loaded!\n");

    return ELF_OK;
}

void elf_unload(const elf_section_offsets_t *info)
{
    const elf_header_t *hdr = (const elf_header_t *)info->file;

    for (uint16_t i = 0; i < hdr->phnum; i++)
    {
        const elf_ph_t *ph = elf_get_ph(info->file, hdr, i);

        if (ph->type != PT_LOAD || ph->memsz == 0)
            continue;

        // tty_printf("\t[ELF] Revoking user privledges at [%d - %d]\n", ph->vaddr, ph->vaddr + ph->memsz);

        paging_clear_user_range(ph->vaddr, ph->memsz);
    }
}

static void elf_jump_user(uint32_t entry, uint32_t stack_top)
{
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    tss_set_esp0(esp);

    __asm__ volatile(
        "mov $0x23, %%ax    \n"
        "mov %%ax,  %%ds    \n"
        "mov %%ax,  %%es    \n"
        "mov %%ax,  %%fs    \n"
        "mov %%ax,  %%gs    \n"

        "push $0x23    \n" // SS
        "push %1       \n" // ESP
        "push $0x202   \n" // EFLAGS
        "push $0x1B    \n" // CS
        "push %0       \n" // EIP
        "iret          \n"
        :
        : "r"(entry), "r"(stack_top)
        : "eax");
}

elf_error_t elf_exec(const uint8_t *file_bytes, uint32_t file_size, uint8_t *user_stack, uint32_t user_stack_size, iret_return_fn_t ret)
{
    elf_section_offsets_t elf_info;

    elf_error_t err = elf_load(file_bytes, file_size, &elf_info);
    if (err != ELF_OK)
    {
        return err;
    }

    uint32_t stack_top = (uint32_t)user_stack + user_stack_size;

    // tty_printf("[ELF] Giving stack access on [%d - %d]\n", user_stack, stack_top);

    paging_set_user_range((uint32_t)user_stack, user_stack_size);

    // tty_printf("[ELF] Jumping to userspace at %d\n", elf_info.entry_fileOff);

    if (setjmp(kernel_return_ctx) == 0)
    {
        elf_jump_user(elf_info.entry_fileOff, stack_top);
    }
    else
    {
        // tty_puts("[ELF] Longjmp return\n");
        elf_unload(&elf_info);
        // tty_puts("[ELF] Unloaded!\n");
        
        interrupts_enable();

        if (ret)
        {
            // tty_printf("[ELF] Calling return function at %d\n", (uint32_t)ret);
            ret();
        }
        else
        {
            asm volatile("cli; hlt");
        }

        return ELF_OK;
    }

    return ELF_OK;
}