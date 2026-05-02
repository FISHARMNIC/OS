/*

This was adapted from a malloc I wrote for class

*/

/*
 * mm-implicit.c -  Simple allocator based on implicit free lists,
 *                  first fit placement, and boundary tag coalescing.
 *
 * Each block has header and footer of the form:
 *
 *      31                     3  2  1  0
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      -----------------------------------
 *
 * where s are the meaningful size bits and a/f is set
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap
 *  -----------------------------------------------------------------
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */

#include <sys/kmalloc.h>
#include <sys/string.h>
#include <graphics.h>
#include <interrupts.h>

/////////////////////////////////////////////////////////////////////////////
// Constants and macros
//
// These correspond to the material in Figure 9.43 of the text
// The macros have been turned into C++ inline functions to
// make debugging code easier.
//
/////////////////////////////////////////////////////////////////////////////
#define WSIZE 4             /* word size (bytes) */
#define DSIZE 8             /* doubleword size (bytes) */
#define CHUNKSIZE (1 << 12) /* initial heap size (bytes) */
#define OVERHEAD 8          /* overhead of header and footer (bytes) */
#define ALIGN(size) (((size) + (DSIZE - 1)) & ~(DSIZE - 1))

static inline uint32_t MAX(uint32_t x, uint32_t y)
{
  return x > y ? x : y;
}

static inline uint32_t MIN(uint32_t x, uint32_t y)
{
  return x < y ? x : y;
}

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
static inline uint32_t PACK(uint32_t size, uint32_t alloc)
{
  return ((size) | (alloc & 0x1));
}

//
// Read and write a word at address p
//
static inline uint32_t GET(void *p) { return *(uint32_t *)p; }
static inline void PUT(void *p, uint32_t val)
{
  *((uint32_t *)p) = val;
}

//
// Read the size and allocated fields from address p
//
static inline uint32_t GET_SIZE(void *p)
{
  return GET(p) & ~0x7;
}

static inline uint32_t GET_ALLOC(void *p)
{
  return GET(p) & 0x1;
}

//
// Given block ptr bp, compute address of its header and footer
//
static inline void *HDRP(void *bp)
{

  return ((char *)bp) - WSIZE;
}
static inline void *FTRP(void *bp)
{
  return ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE);
}

//
// Given block ptr bp, compute address of next and previous blocks
//
static inline void *NEXT_BLKP(void *bp)
{
  return ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)));
}

static inline void *PREV_BLKP(void *bp)
{
  return ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)));
}

#define dbg_printf(...) // dbg_printf(__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//

static char *heap_listp; /* pointer to first block */

/* seglists
Each one of these lists is the root pointer to an explicit free list
Seglists are:
0-15
16-31
32-63
64-127
...
UINT max

The explicit free lists are going to be LIFO order such that frees are as simple as appending to head

coalese:
  suppose current block A was just freed and neighboring block B is free
  * Use boundary tags to get address of neighbor
  * Neighbors addr can now be used to O(1) it in its according LL
  * Merge both blocks and get new size (already implemented)
  * Remove both blocks from their free list
  * Add new block into acording free list

free:
  * Get size of block, append to according free list
    -> use empty info (first 2 dwords) in block
    -> enough to store prev and next ptr
    -> insert as head, so:
      new->next = head
      head->prev = new
      head = new

find_fit:
  * Get header of according free list
  * Traverse free list until next = NULL, using boundary tags for size

malloc:
  * Get address using find_fit
  * Using struct at address, O(1) it in the LL
  * Merge its previous and next to remove it from the list

*/
typedef struct dbll
{
  struct dbll *prev;
  struct dbll *next;
} dbll_t;

static dbll_t *seglists[30] = {NULL};
static dbll_t *contptr[30] = {NULL};

static inline uint32_t MSB(uint32_t i)
{
  return (31 - __builtin_clz(i));
}

uint32_t seglist_get_group(uint32_t x)
{
  // x = MIN(1048576, x);
  if (x <= 15)
    return 0;
  return MSB(x) - 3;
}

void seglist_append(uint32_t size, dbll_t *address)
{
  uint32_t index = seglist_get_group(size);

  dbll_t *chead = seglists[index];

  // if (chead == address) // was looping for some reason. whatevs
  //   return;

  dbg_printf("%d, Appending[%d], %p. Was %p\n", size, index, address, chead);

  // prepare for insertion as head
  // prev -> nothing
  // next -> current_head
  address->prev = NULL;
  address->next = chead;
  if (chead)
    chead->prev = address;

  // insert as head
  seglists[index] = address;
}

void seglist_remove(dbll_t *address)
{
  dbll_t *left = address->prev;
  dbll_t *right = address->next;

  if (left)
    left->next = right;

  if (right)
    right->prev = left;

  uint32_t i = seglist_get_group(GET_SIZE(HDRP(address)));

  if (seglists[i] == address) // head
  {
    seglists[i] = right;
    contptr[i] = right;
  }
  else if (contptr[i] == address)
  {
    contptr[i] = right;
  }
  // contptr[i] = seglists[i];

  address->prev = NULL;
  address->next = NULL;
}

void seglist_remove_old(dbll_t *address)
{
  dbll_t *left = address->prev;
  dbll_t *right = address->next;

  // dbg_printf("Removing %p from [%d]. LR: %p %p\n", address, seglist_get_group(GET_SIZE(HDRP(address))), left, right);

  if (left)
  {
    left->next = right;
  }
  else // removed head
  {
    seglists[seglist_get_group(GET_SIZE(HDRP(address)))] = right;
  }

  if (right)
  {
    right->prev = left;
  }

  address->prev = NULL;
  address->next = NULL;
}

//
// function prototypes for internal helper routines
//
static void *extend_heap(uint32_t words);
static void place(void *bp, uint32_t asize);
static void *find_fit(uint32_t asize);
static void *coalesce(void *bp);
// static void printblock(void *bp);
// static void checkblock(void *bp);

//
// mm_init - Initialize the memory manager
//
uint32_t mm_init(void)
{

  /*
  sbrk(_sbytes) increments the heap by size _sbytes and returns the old heap address, such that
  there are _sbytes free bytes after its return value

  This basically just "allocates" space for 4 words and gets the addres of the start of that space
  */

  for (uint32_t i = 0; i < 30; i++)
  {
    seglists[i] = NULL;
    contptr[i] = NULL;
  }

  heap_listp = mem_sbrk(4 * WSIZE);

  if (heap_listp == (void *)-1) // if something didn't work, return status
  {
    return -1;
  }

  // initial padding, since: (word) padding + (word) header = 4 + 4 = 8. All blocks start at 8-aligned
  PUT(heap_listp, 0);

  // prologue block, starting point for heap
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header, stores (size | alloc_bit)
                                                 // used when looking forwards from block behind
                                                 // 1 = already taken. This block can never be "freed" as its just the marker
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footder, stores THE SAME (size | alloc_bit)
                                                 // used when looking backwards from a block infront
                                                 // makes it so you dont have to walk the heap from front->back just to get the previous blocks info

  // epilogue block. This is where the allocated area ends.
  // Further sbrks overwrite this with the next blocks prologue header
  // Epilogue has info (size | free?) = (0, notFree)
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

  // skip padding to get right between the prologue header and footer, where the first real block will be
  // @heap_listp is currently the footer
  // @(heap_listp - WSIZE) is the header
  heap_listp += 2 * WSIZE;

  // create block after the header
  void *rp = extend_heap(CHUNKSIZE / WSIZE);

  if (rp == NULL)
    return -1;

  return 0;
}

//
// extend_heap - Extend heap with free block and return its block pointer
//
static void *extend_heap(uint32_t words)
{
  // want an even number of 4-byte words, such that its always aligned to 8-bytes
  words += words & 1;

  uint32_t bytes = words * WSIZE;

  // uint32_t bytes = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

  /*
  The break (between data and heap) is currently at the epilogue (as done by the last sbrk)
    -> for example, after mm_init does sbrk with 4 words, the break points at that border: old_break + (4 * WISE)
  This means that this sbrk allocates space right after the epilogue.

  Hence:
    address(alloc)          = one word right after the old epilogue
    address(alloc - WISE)   = address of old epilogue => soon becomes the new blocks prologue header
    address(alloc - 2*WISE) = address of last blocks footer
  */
  void *alloc = mem_sbrk(bytes); // allocate heap with desired number of bytes

  if (alloc == (void *)-1) // sbrk error
  {
    return NULL;
  }

  PUT(HDRP(alloc), PACK(bytes, 0)); // New blocks prologue header with info: (bytes | 0) means (blockSize | isFree)
                                    // As mentioned above, this was the old epilogue
  PUT(FTRP(alloc), PACK(bytes, 0)); // Same block, but this is the info at the end of it (footer), for the next block to look back at it

  PUT(HDRP(NEXT_BLKP(alloc)), PACK(0, 1)); // Mark the new epilogue (size 0, notFree)

  // alloc = coalesce(alloc);
  seglist_append(GET_SIZE(HDRP(alloc)), alloc);

  return alloc;
}

static void *find_fit(uint32_t asize)
{
  uint32_t i = seglist_get_group(asize);
  if (contptr[i] == NULL)
  {
    contptr[i] = seglists[i];
  }
  void *bp = contptr[i];

  void *headPtr;

  while (i < 29) // when size = 0, it reached the epilogue
  {
    // dbg_printf("finding fit :: %d :: %p\n", i, bp);
    if (bp == NULL)
    {
      contptr[i] = seglists[i];
      i++;
      if (contptr[i] == NULL)
        contptr[i] = seglists[i];
      bp = contptr[i];
    }
    else
    {

      headPtr = HDRP(bp);

      // uint32_t isFree = !GET_ALLOC(headPtr);
      uint32_t bsize = GET_SIZE(headPtr);

      if (!GET_ALLOC(HDRP(bp)) && bsize >= asize) // if not free, and the block size is enough to hold the alloc size
      {
        return bp;
      }

      bp = ((dbll_t *)bp)->next; // jump to next block and repeat
    }
  }

  return NULL; // nothing found
}

//
// kfree - Free a block
//
void kfree(void *bp)
{
  /*
  All this does is mark a block as freed by clearing it alloc bit
  */

  // tty_printf("Free %d\n", bp);

  uint32_t currentInfo = GET(HDRP(bp)); // get the current header info (just care about the size)

  // header and footer contain the size packed with the alloc bit
  // and with -1 = all bits same except bit 0 => 0, marking the alloc bit as "free"
  PUT(HDRP(bp), currentInfo & ~1); // mark it as free
  PUT(FTRP(bp), currentInfo & ~1); // mark footer as free

  // todo optimize all this HDRP stuff
  // seglist_append(GET_SIZE(HDRP(bp)), bp);

  void *coal = coalesce(bp);

  seglist_append(GET_SIZE(HDRP(coal)), coal);
}

//
// coalesce - boundary tag coalescing. Return ptr to coalesced block
//
static void *coalesce(void *bp)
{
  /*
  Coalescing converts adjacent free blocks into one larger free block
  There are 4 cases to handle:
    1. Nothing around bp is free -> cant do anything
    2. Only next block is free   -> merge bp with next
    3. Only prev block is free   -> merge prev with bp
    4. Both blocks are free      -> merge prev with next

  Note: bp is always free, as in nothing is using it
  */
  void *prevBlock = PREV_BLKP(bp);
  void *nextBlock = NEXT_BLKP(bp);

  uint32_t lastIsntFree = GET_ALLOC(FTRP(prevBlock));
  uint32_t nextIsntFree = GET_ALLOC(HDRP(nextBlock));

  if (nextIsntFree && lastIsntFree) // nothing can be done, just two filled blocks
  {
    return bp;
  }
  else if (lastIsntFree && !nextIsntFree) // last isnt free but the next one is
  {

    seglist_remove(nextBlock);
    // seglist_remove(bp);

    /*
    In this case, merge the free block at bp with the next block, which is also free
    This makes it so that instead of having two smaller blocks (bp, next), its now one bigger block
      -> which is of size: size(bp) + size(next)
    So chain bp header -> next footer
    */
    uint32_t newSize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(nextBlock));

    PUT(HDRP(bp), PACK(newSize, 0));
    PUT(FTRP(nextBlock), PACK(newSize, 0));

    /*
    Note that the next blocks info doesn't need to be "removed" since this blocks info
    forces the other ones to be skipped over, as when chaining through the list,
    both the header and footer jump over the over block, basically ghosting it.
    */

    return bp;
  }
  else if (!lastIsntFree && nextIsntFree) // last is free but next isnt
  {

    seglist_remove(prevBlock);

    /*
    Similar to case above this, except merges the previous block with the current one
    */

    uint32_t newSize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(prevBlock));

    // make the previous block header say to end at the new size
    // and make the current blocks footer say it came from the new size
    PUT(HDRP(prevBlock), PACK(newSize, 0));
    PUT(FTRP(bp), PACK(newSize, 0));

    // above two lines ghost prev footer and bp header, such that chaining makes it skip over bp
    // so it goes prev header -> bp footer

    return prevBlock;
  }
  else // last and next are free
  {
    seglist_remove(prevBlock);
    seglist_remove(nextBlock);

    // similar to above, but blocks to the left and right are free, so skip over bp entirely
    void *prevHeaderp = HDRP(prevBlock);
    void *nextFooterp = FTRP(nextBlock);

    uint32_t newSize = GET_SIZE(prevHeaderp) + GET_SIZE(HDRP(bp)) + GET_SIZE(nextFooterp);

    PUT(prevHeaderp, PACK(newSize, 0));
    PUT(nextFooterp, PACK(newSize, 0));

    return prevBlock;
  }
  return bp;
}

//
// kmalloc - Allocate a block with at least size bytes of payload
//

/*
 is the issue that when taking up free (mallocing) it doesnt notify that a neigbboring block is now occup



*/

void *kmalloc(uint32_t size)
{
  // tty_printf("Mallocing %d ", size);
  if (size == 0 || size >= (UINT32_MAX - 4 * DSIZE))
  {
    tty_reset();
    tty_printf("[ERROR] Malloc Failiure - SIZE\n");
    panic();
    return NULL;
  }

  uint32_t payloadSize = size;

  if (size <= DSIZE)
  {
    size = 4 * DSIZE; // if less than 8 bytes, force 8b alignment
    // The header and footer are each 4 bytes (one word)
    // So 1 doubleword is for user data, 1 double word is for the 2 prologue words
  }
  else
  {
    size = ALIGN(size + OVERHEAD);
  }

  void *attemptedFit = find_fit(size);

  dbg_printf("*");

  if (attemptedFit == NULL) // no found block
  {
    // TODO maybe make bigger than this? but 1 << 12 is alread 4096 so maybe not
    if ((attemptedFit = extend_heap(MAX(size, CHUNKSIZE) / WSIZE)) == NULL) // extend the heap to a new size, and place fit as new empty slot
    {
      tty_reset();
      tty_printf("[ERROR] Malloc Failiure - SBRK\n");
      panic();
      return NULL; // sbrk error
    }
  }

  dbg_printf("*");

  // place
  place(attemptedFit, size);
  dbg_printf("*\n");

  // tty_printf("Malloc %d bytes at %d\n", size, attemptedFit); // @todo notice first alloc gets leaked? after that all have same addr

  memset(attemptedFit, 0, payloadSize);
  return attemptedFit;
}

//
//
// Practice problem 9.9
//
// place - Place block of asize bytes at start of free block bp
//         and split if remainder would be at least minimum block size
//
static void place(void *bp, uint32_t asize)
{
  void *bhdrp = HDRP(bp);
  uint32_t blockSize = GET_SIZE(bhdrp);

  // get rid of free block
  seglist_remove(bp);

  uint32_t rem = blockSize - asize;
  // TODO this might need to be made bigger
  if (rem >= (4 * DSIZE)) // if the remainder block is big enough to be a block on its own
  {
    /*
    If block is bigger than needed, and theres enough space for prologues
      -> then fit another free block after this block and before the next block
    */
    PUT(bhdrp, PACK(asize, 1));    // insert new size
    PUT(FTRP(bp), PACK(asize, 1)); // end of new allocated block

    bp = NEXT_BLKP(bp);

    PUT(HDRP(bp), PACK(rem, 0)); // start of new free remainder block
    PUT(FTRP(bp), PACK(rem, 0)); // end of new free remainder block

    // add new, split free block
    seglist_append(rem, bp);

    // void* coal = coalesce(bp);
    // seglist_append(GET_SIZE(HDRP(coal)), coal);
  }
  else // cant meander another free block
  {
    // mark as taken
    PUT(HDRP(bp), PACK(blockSize, 1));
    PUT(FTRP(bp), PACK(blockSize, 1));
  }
}

//
// mm_realloc -- implemented for you
//
void *mm_realloc(void *ptr, uint32_t size)
{
  dbg_printf("Reallocing %p size %d ", ptr, size);

  void *newp;
  uint32_t copySize;

  newp = kmalloc(size);

  dbg_printf("*");
  if (newp == NULL)
  {
    dbg_printf("ERROR: kmalloc failed in mm_realloc\n");
    return NULL; // @todo kernel fault
  }
  copySize = GET_SIZE(HDRP(ptr));

  if (size < copySize)
  {
    copySize = size;
  }
  memcpy(newp, ptr, copySize);
  dbg_printf("*");
  kfree(ptr);
  dbg_printf("*\n");
  return newp;
}

//
// mm_checkheap - Check the heap for consistency
//
// void mm_checkheap(uint32_t verbose)
// {
//   //
//   // This provided implementation assumes you're using the structure
//   // of the sample solution in the text. If not, omit this code
//   // and provide your own mm_checkheap
//   //
//   void *bp = heap_listp;

//   if (verbose)
//   {
//     dbg_printf("Heap (%p):\n", heap_listp);
//   }

//   if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
//   {
//     dbg_printf("Bad prologue header\n");
//   }

//   checkblock(heap_listp);

//   for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
//   {
//     if (verbose)
//     {
//       printblock(bp);
//     }
//     checkblock(bp);
//   }

//   if (verbose)
//   {
//     printblock(bp);
//   }

//   if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
//   {
//     dbg_printf("Bad epilogue header\n");
//   }
// }

// static void printblock(void *bp)
// {
//   uint32_t hsize, halloc, fsize, falloc;

//   hsize = GET_SIZE(HDRP(bp));
//   halloc = GET_ALLOC(HDRP(bp));
//   fsize = GET_SIZE(FTRP(bp));
//   falloc = GET_ALLOC(FTRP(bp));

//   if (hsize == 0)
//   {
//     dbg_printf("%p: EOL\n", bp);
//     return;
//   }

//   dbg_printf("%p: header: [%d:%c] footer: [%d:%c]\n",
//          bp,
//          (uint32_t)hsize, (halloc ? 'a' : 'f'),
//          (uint32_t)fsize, (falloc ? 'a' : 'f'));
// }

// static void checkblock(void *bp)
// {
//   if ((uintptr_t)bp % 8)
//   {
//     dbg_printf("Error: %p is not doubleword aligned\n", bp);
//   }
//   if (GET(HDRP(bp)) != GET(FTRP(bp)))
//   {
//     dbg_printf("Error: header does not match footer\n");
//   }
// }
