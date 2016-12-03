/* Tar Heels Allocator
 *
 * Simple Hoard-style malloc/free implementation.
 * Not suitable for use for large allocatoins, or
 * in multi-threaded programs.
 *
 * to use:
 * $ export LD_PRELOAD=/path/to/th_alloc.so <your command>
 */

/* Hard-code some system parameters */

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_MASK (~(SUPER_BLOCK_SIZE-1))  // fffff000
#define MIN_ALLOC 32 /* Smallest real allocation.  Round smaller mallocs up */
#define MAX_ALLOC 2048 /* Fail if anything bigger is attempted.
* Challenge: handle big allocations */
#define RESERVE_SUPERBLOCK_THRESHOLD 2

#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#define assert(cond) if (!(cond)) __asm__ __volatile__ ("int $3")

/* Object: One return from malloc/input to free. */
struct __attribute__((packed)) object {
    union {
        struct object *next; // For free list (when not in use)
        char * raw; // Actual data
    };
};


/* Super block bookeeping; one per superblock.  "steal" the first
 * object to store this structure
 */

struct __attribute__((packed)) superblock_bookkeeping {
    struct superblock_bookkeeping * next; // next super block
    struct object *free_list;
    // Free count in this superblock
    uint8_t free_count; // Max objects per superblock is 128-1, so a byte is sufficient
    uint8_t level;
};



/* Superblock: a chunk of contiguous virtual memory.
 * Subdivide into allocations of same power-of-two size. */
struct __attribute__((packed)) superblock {
    struct superblock_bookkeeping bkeep;
    void *raw;  // Actual data here
};


/* The structure for one pool of superblocks.
 * One of these per power-of-two */
struct superblock_pool {
    struct superblock_bookkeeping *next;
    uint64_t free_objects; // Total number of free objects across all superblocks
    uint64_t whole_superblocks; // Superblocks with all entries free
};


// 10^5 -- 10^11 == 7 levels
#define LEVELS 7
static struct superblock_pool levels[LEVELS] = {
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0}
};

static inline int size2level (ssize_t size) {
    /* Your code here.
     * Convert the size to the correct power of two.
     * Recall that the 0th entry in levels is really 2^5,
     * the second level represents 2^6, etc.
     */

    int res = 0;
    if(1024<size && size<=2048) res=6;
    else if(size<=1024&&size>512) res=5;
    else if(size<=512&&size>256)  res=4;
    else if(size<=256&&size>128)  res=3;
    else if(size<=128&&size>64)   res=2;
    else if(size<=64&&size>32)   res=1;
    else res=0;
    printf("%d\n", res);
    return res;   
}
static inline
struct superblock_bookkeeping * alloc_super (int power) {
    //initialize page here to use
    void *page;
    struct superblock* sb;
    int free_objects = 0, bytes_per_object = 0;
    char *cursor;
    
    // Your code here
    // Allocate a page of anonymous memory
    // WARNING: DO NOT use brk---use mmap, lest you face untold suffering
    
    page = mmap(0, SUPER_BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    
    sb = (struct superblock*) page;
    // Put this one the list.
    sb->bkeep.next = levels[power].next;
    levels[power].next = &sb->bkeep;
    levels[power].whole_superblocks++;
    sb->bkeep.level = power;
    sb->bkeep.free_list = NULL;
    
    // Your code here: Calculate and fill the number of free objects in this superblock
    //  Be sure to add this many objects to levels[power]->free_objects, reserving
    //  the first one for the bookkeeping.
    
    bytes_per_object = 1<<(power+5);
    
    int offset = 1<<power;
    free_objects = 128/offset-1;     //number of free objs in this superblcok
    sb->bkeep.free_count = free_objects;
	printf("initial free_count %d\n", sb->bkeep.free_count);
    levels[power].free_objects += free_objects;
    
    
    
    // The following loop populates the free list with some atrocious
    // pointer math.  You should not need to change this, provided that you
    // correctly calculate free_objects.
    
    cursor = (char *) sb;
    // skip the first object
    for (cursor += bytes_per_object; free_objects--; cursor += bytes_per_object) {
        // Place the object on the free list
        struct object* tmp = (struct object *) cursor;
        tmp->next = sb->bkeep.free_list;
        sb->bkeep.free_list = tmp;
    }
    return &sb->bkeep;
}

void printlevels()
{
  int i;
  for(i=0; i<LEVELS; i++)
  {
	printf("The %d level: %d wholeSB %d freeobjects\n", i, levels[i].whole_superblocks, levels[i].free_objects);
  }
}


void *malloc(size_t size) {
    struct superblock_pool *pool;
    struct superblock_bookkeeping *bkeep;
    void *rv = NULL;
    int power = size2level(size);
    int offset = 1<<power;
    
    // Check that the allocation isn't too big
    if (size > MAX_ALLOC) {
        errno = -ENOMEM;
        return NULL;
    }
    
    // Delete the following two lines
    // errno = -ENOMEM;
    // return rv;    
    pool = &levels[power];
    
    if (!pool->free_objects) {
        bkeep = alloc_super(power);
    }
    else
        bkeep = pool->next;
    
    //printf("%d\n" , offset);
    while (bkeep != NULL) {
        if (bkeep->free_count) { //free_count is not 0
            
            struct object *next = bkeep->free_list;
            /* Remove an object from the free list. */
            // Your code here
            //
            // NB: If you take the first object out of a whole
            //     superblock, decrement levels[power]->whole_superblocks
            
            if( bkeep->free_count == 128/(offset)-1 ){ //this is a whole-superblock //ruibin
			    //printf("malloc - whole superblocks-1\n");
                levels[power].whole_superblocks -=1;
            }
            

            bkeep->free_list = next->next;
            bkeep->free_count -=1;
            levels[power].free_objects--;
            
            rv = next;
            

            break;
        }
        bkeep = bkeep->next; // otherwise, go to next superblock
    }
    printlevels();
    // assert that rv doesn't end up being NULL at this point
    assert(rv != NULL);
    
    /* Exercise 3: Poison a newly allocated object to detect init errors.
     * Hint: use ALLOC_POISON
     */
    memset(rv, ALLOC_POISON , 1<<(power+5) );
    return rv;
}




static inline
struct superblock_bookkeeping * obj2bkeep (void *ptr) {
    uint64_t addr = (uint64_t) ptr;
    addr &= SUPER_BLOCK_MASK;
    return (struct superblock_bookkeeping *) addr;
}



void free(void *ptr) {
    struct superblock_bookkeeping *bkeep = obj2bkeep(ptr);
    
    // Your code here.
    //   Be sure to put this back on the free list, and update the
    //   free count.  If you add the final object back to a superblock,
    //   making all objects free, increment whole_superblocks.
    

    //add to the front of free-list
    struct object* newObj = (struct object *) ptr;
    newObj->next = bkeep->free_list;
    bkeep->free_list = newObj;
    //update statistics

    int currentLevel = bkeep->level;
    int max = 128/(1<<currentLevel)-1;
    if( (++bkeep->free_count) == max ){
        levels[currentLevel].whole_superblocks++;
    }
    levels[currentLevel].free_objects += 1; 
    memset(ptr, FREE_POISON , 1<<(currentLevel+5));

    while (levels[currentLevel].whole_superblocks > RESERVE_SUPERBLOCK_THRESHOLD) {
        // Exercise 4: Your code here
        // Remove a whole superblock from the level
        // Return that superblock to the OS, using mmunmap
        struct superblock_bookkeeping *node = levels[currentLevel].next;
        levels[currentLevel].next = levels[currentLevel].next->next;
        if(node->free_count== 128/(1<<currentLevel)-1 )
		{
		  //remove current node2, and 
          //update freelist and bkeep LinkedList
          //then munmap
          levels[currentLevel].whole_superblocks--;
          levels[currentLevel].free_objects -= max;
          munmap(node, SUPER_BLOCK_SIZE); //ruibinma
        }
    printf("returned a page to OS\n");
    // break; // hack to keep this loop from hanging; remove in ex 4
	} 

    /* Exercise 3: Poison a newly freed object to detect use-after-free errors.
     * Hint: use FREE_POISON
     */
	printlevels();
}



// Do NOT touch this - this will catch any attempt to load this into a multi-threaded app
int pthread_create(void __attribute__((unused)) *x, ...) {
    exit(-ENOSYS);
}
