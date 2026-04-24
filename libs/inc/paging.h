#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

void paging_init(void);
void paging_set_user(uint32_t vaddr);
void paging_clear_user(uint32_t vaddr);
void paging_set_user_range(uint32_t vaddr, uint32_t size);
void paging_clear_user_range(uint32_t vaddr, uint32_t size);

#endif