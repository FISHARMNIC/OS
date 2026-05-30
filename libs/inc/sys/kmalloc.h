/*

This was adapted from a malloc I had to wrote for class

*/



#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>

extern uint32_t mm_init (void);
extern void *kmalloc (uint32_t size);
extern void kfree (void *ptr);
extern void *mm_realloc(void *ptr, uint32_t size);

void mem_init(void);               
// void mem_deinit(void);
void *mem_sbrk(uint32_t incr);
void mem_reset_brk(void); 
void *mem_heap_lo(void);
void *mem_heap_hi(void);
uint32_t mem_heapsize(void);

void mm_checkheap();
// uint32_t mem_pagesize(void);

/* 
 * Alignment requirement in bytes (either 4 or 8) 
 */
#define ALIGNMENT 8  

/* 
 * Maximum heap size in bytes 
 */
#define MAX_HEAP (1024 * 1024 * 100)

#define NULL ((void*) 0)



#endif