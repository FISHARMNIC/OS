#include <paging.h>

#define PAGE_PRESENT  0x001U
#define PAGE_RW       0x002U
#define PAGE_USER     0x004U
#define PAGE_4MB      0x080U

#define CR0_PG  (1U << 31)
#define CR4_PSE (1U << 4)

#define PDE_INDEX(vaddr)    ((vaddr) >> 22)
#define ALIGN_4MB(vaddr)    ((vaddr) & ~((1U << 22) - 1U))

static uint32_t page_directory[1024] __attribute__((aligned(4096)));

void paging_init(void)
{
    uint32_t i;
    uint32_t cr0;
    uint32_t cr4;

    // Identity-map full 4G in 4MB chunks, kernel-only by default (no PAGE_USER)
    for (i = 0; i < 1024U; ++i) {
        page_directory[i] = (i << 22) | PAGE_PRESENT | PAGE_RW | PAGE_4MB;
    }

    __asm__ volatile("mov %0, %%cr3" : : "r"(&page_directory[0]) : "memory");

    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= CR4_PSE;
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= CR0_PG;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

// Ring 3
void paging_set_user(uint32_t vaddr)
{
    uint32_t idx = PDE_INDEX(vaddr);
    page_directory[idx] |= PAGE_USER;
    __asm__ volatile("invlpg (%0)" : : "r"(ALIGN_4MB(vaddr)) : "memory");
}

// Revoke Ring3 access
void paging_clear_user(uint32_t vaddr)
{
    uint32_t idx = PDE_INDEX(vaddr);
    page_directory[idx] &= ~PAGE_USER;
    __asm__ volatile("invlpg (%0)" : : "r"(ALIGN_4MB(vaddr)) : "memory");
}

// Mark every 4MB region covered by [vaddr, vaddr+size) as user-accessible.
void paging_set_user_range(uint32_t vaddr, uint32_t size)
{
    uint32_t addr = ALIGN_4MB(vaddr);
    uint32_t end  = vaddr + size;

    while (addr < end) {
        paging_set_user(addr);
        addr += (1U << 22); /* advance one 4MB page */
    }
}

// Revoke user access for every 4MB region covered by [vaddr, vaddr+size).
void paging_clear_user_range(uint32_t vaddr, uint32_t size)
{
    uint32_t addr = ALIGN_4MB(vaddr);
    uint32_t end  = vaddr + size;

    while (addr < end) {
        paging_clear_user(addr);
        addr += (1U << 22);
    }
}