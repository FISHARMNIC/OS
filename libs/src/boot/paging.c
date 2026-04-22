#include <paging.h>

#define PAGE_PRESENT 0x001U
#define PAGE_RW 0x002U
#define PAGE_4MB 0x080U

#define CR0_PG (1U << 31)
#define CR4_PSE (1U << 4)

// Single page directory used with 4M pages (no separate page tables)
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

void paging_init(void)
{
    uint32_t i;
    uint32_t cr0;
    uint32_t cr4;

    // Map the full 4G virtual range in 4M chunks
    for (i = 0; i < 1024U; ++i) {
        page_directory[i] = (i << 22) | PAGE_PRESENT | PAGE_RW | PAGE_4MB;
    }

    // Tell CPU where the page directory lives
    __asm__ volatile("mov %0, %%cr3" : : "r"(&page_directory[0]) : "memory");

    // Enable page size extension so PDE entries can map 4M pages directly
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= CR4_PSE;
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    // cr0 enable paging bit
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= CR0_PG;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
