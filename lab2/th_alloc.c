
/* UNC Honor Pledge: 
 *      I certify that no unauthorized assistance has been received or 
 *   given in the completion of this work.
 *    
 *   Qidi Chen,  Ruibin Ma
 *
 */

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_MASK (~(SUPER_BLOCK_SIZE-1))
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

//convert size to level
static inline int size2level (ssize_t size) {
     
    int res = 0;

    if(1024<size && size<=2048) res=6;
    else if(size<=1024&&size>512) res=5;
    else if(size<=512&&size>256)  res=4;
    else if(size<=256&&size>128)  res=3;
    else if(size<=128&&size>64)   res=2;
    else if(size<=64&&size>32)   res=1;
    else res=0;

    return res;

}


static inline
struct superblock_bookkeeping * alloc_super (int power) {
    //initialize page here to use
    void *page;
    struct superblock* sb;
    
    char *cursor;
    
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
    
    int free_objects = 0, bytes_per_object = 0;
    bytes_per_object = 1<<(power+5);

    int offset;
    offset = 1<<power; // 2^power

    //number of free objs in this superblcok
    free_objects = 128/offset-1;    

    //update statistics
    sb->bkeep.free_count = free_objects;
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

/*  
    For 1st challenge problem.
 
    I Define a class/struct to store information about
    address and size of a newly allocated memory block
    that is larger than 2KB。
 
*/
struct __attribute__((packed))  area { 
    struct area * next;
    void *addr;
    int size1;
    
};

//Initialize the head of the “Linked-List”
struct area start = {NULL, NULL, 0} ; 


//Another malloc designed for allocation >= 2KB
void *malloc2(size_t size){ 
    
    void *page2;
    //Reserve extra memory for "struct area"
    page2 = mmap(0, size+sizeof(struct area), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    
    //nb: new memory block
    struct area* nb = (struct area*) page2;
    //Save the starting address of new page
    nb->addr = (char*)page2+sizeof(struct area);
    //Save the size of new malloc into 
    nb->size1 = size;
    //Add this new block in the "Linked-List"
    if(start.next == NULL){
        start.next = nb;
    }
    else{
        nb->next = start.next;
        start.next = nb;
    }

    //return the actual address of memory
    memset((char*)page2+sizeof(struct area), ALLOC_POISON, size);
    return (char*)page2+sizeof(struct area);
}


void *malloc(size_t size) {
    
    //Handle big allocation
    if(size>2048){ 

        return malloc2(size);
    }   
    
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
    
    pool = &levels[power];
    
    if (!pool->free_objects) {
        bkeep = alloc_super(power);
    }
    
    else
        bkeep = pool->next;

 
    while (bkeep != NULL) {

        // If free_count is not 0
        if (bkeep->free_count) { 
            struct object *next = bkeep->free_list;
            /* Remove an object from the free list. */

            /*
             if this is a whole-superblock, then
             decrement levels[power]->whole_superblocks
             */
            if( bkeep->free_count == 128/(offset)-1 ){ 

                levels[power].whole_superblocks -=1;
            }

            //Update the free_list
            bkeep->free_list = next->next;
            bkeep->free_count -=1;
            levels[power].free_objects--;
            rv = next;
            break;
        }

        // otherwise, go to next superblock
        bkeep = bkeep->next; 
    }

    // assert that rv doesn't end up being NULL at this point
    assert(rv != NULL);
    
    //Poison 
    memset(rv, ALLOC_POISON , 1<<(power+5) );

    return rv;
}




static inline
struct superblock_bookkeeping * obj2bkeep (void *ptr) {
    uint64_t addr = (uint64_t) ptr;
    addr &= SUPER_BLOCK_MASK;
    return (struct superblock_bookkeeping *) addr;
}

// Free function for large memory
int findLargeFree(void *ptr){
    
    /*
     Node of Linked-List.
     "start" is a global dummy head node.
    */
    struct area* node = start.next;

    //Try to find a memory match
    while(node!=NULL){
        if(node->addr == ptr){ // Found
            //return size of allocated memory
            return (*node).size1;
        }
        node = node->next;
    }

    //Not found, return 0
    return 0;
}

uint8_t upper = 6;
uint8_t lower = 0;

void free(void *ptr) {

    if(ptr == NULL)
        return;
    
    struct superblock_bookkeeping *bkeep = obj2bkeep(ptr);
    
    //Check whether this ptr is in any SuperBlock Level
    if(bkeep==NULL||(bkeep->level<lower||bkeep->level>upper)){
        // ptr not in any SuperBlock Level
        int largeMemorySize;
        largeMemorySize = findLargeFree(ptr);

        if(largeMemorySize != 0){
            munmap((char*)ptr-sizeof(struct area),largeMemorySize+sizeof(struct area));
            return ;
        }
    }
    /*
     Now we know ptr is in SuperBlock level
     */
    int currentLevel ;
    int capacity;
    
    //add to the front of free-list
    struct object* newObj = (struct object *) ptr;
    newObj->next = bkeep->free_list;
    bkeep->free_list = newObj;

    //update statistics
    
    currentLevel = bkeep->level;
    capacity = 128/(1<<currentLevel)-1; // capacity of this level
    
    //update whole_superblock
    if( (++bkeep->free_count) == capacity ){
        levels[currentLevel].whole_superblocks++;
    }

    levels[currentLevel].free_objects += 1;

    //Free Poison
    memset( (char*) ptr + 8, FREE_POISON , (1<<(currentLevel+5)) - 8); 
    
    //Return SB to OS

    struct superblock_bookkeeping *lastnode = levels[currentLevel].next;

    struct superblock_bookkeeping *currentnode = levels[currentLevel].next;
    
    while (levels[currentLevel].whole_superblocks > RESERVE_SUPERBLOCK_THRESHOLD) {
        
        struct superblock_bookkeeping *node = currentnode;
        currentnode = currentnode->next;

        if(node->free_count== 128/(1<<currentLevel)-1 )
            {
                //Found a whole_superblock
                levels[currentLevel].whole_superblocks--;
                levels[currentLevel].free_objects -= capacity;
                if(node == levels[currentLevel].next)
                {
                    levels[currentLevel].next = currentnode;
                }
                else
                {
                    lastnode->next = currentnode;
                }
                //Return to OS
                munmap(node, SUPER_BLOCK_SIZE);    

        }

        else
            {
                lastnode = node;
        }

    }
    
    //End of Free()
}



// Do NOT touch this - this will catch any attempt to load this into a multi-threaded app
int pthread_create(void __attribute__((unused)) *x, ...) {
    exit(-ENOSYS);
}

