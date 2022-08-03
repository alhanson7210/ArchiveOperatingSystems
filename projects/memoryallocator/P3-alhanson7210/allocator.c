/**
 * @file allocator.c
 *
 * Explores memory management at the C runtime level.
 *
 * @brief To use (one specific command):
 * LD_PRELOAD=$(pwd)/allocator.so command
 * ('command' will run with your allocator)
 *
 * To use (all following commands):
 * export LD_PRELOAD=$(pwd)/allocator.so
 * (Everything after this point will use your custom allocator -- be careful!)
 */

/*
 * list of finished methods(coding logic)
 * malloc
 * print_memory
 * calloc
 * reuse
 * malloc_name
 * free
 * realloc
 *
 * syntactical check
 * TODO: print_memory reuse malloc malloc_name free calloc realloc
 * 
 * TODO: ask about here
 * (3) in malloc_name
 * (1) in free
 * (1) in calloc
 * (1) in realloc
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include "allocator.h"
#include "debug.h"

// TODO: add thread safety
pthread_mutex_t thread_safety = PTHREAD_MUTEX_INITIALIZER;

/**
 * print_memory prints the current memory state from our linked list of memory
 * @param void 
 * @brief Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 */
void print_memory(void)
{
    fprintf(stdout, "-- Current Memory State --\n");
    struct mem_block * current_block = g_head;
    struct mem_block * current_region = NULL;
    while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            fprintf(stdout, "[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        fprintf(stdout, "[BLOCK]  %p-%p (%lu) '%s' %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->name,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}

/**
 * write_memory prints the current memory state from our linked list of memory without calling malloc
 * @param fp file pointer
 * @brief Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 */
void write_memory(FILE * fp)
{
    fprintf(fp, "-- Current Memory State --\n");
    struct mem_block * current_block = g_head;
    struct mem_block * current_region = NULL;
    while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            fprintf(fp, "[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        fprintf(fp, "[BLOCK]  %p-%p (%lu) '%s' %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->name,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}

/**
 * Resusing blocks of memory within our linked list of memory 
 * @param size requested allocation size
 * @brief reuse returns the first memory region
 * that the requested allocation size fits
 * into -which includes the size fo the 
 * mem_block struct that will be split
 */
void *reuse(size_t size)
{
    if(g_head == NULL) return NULL;

    // TODO: using free space management algorithms, find a block of memory that
    // we can reuse. Return NULL if no suitable block is found.
    // use first_fit by default if there is no algorithm given
    char * algo = getenv("ALLOCATOR_ALGORITHM");

    if (algo == NULL) 
    {
         algo = "first_fit";
    }

    if (strcmp(algo, "first_fit") == 0) 
    {

        /* first fit block */
        struct mem_block * current = g_head;

        while (current != NULL)
        {
            //find the first actual free available amount of memory within a given region
            size_t free_space = (current -> size) - (current -> usage);

            //aligning the memory allocation
            size_t requested = size + sizeof(struct mem_block);

            if (free_space >= requested) 
            {
                LOG("reuse -> first_fit: returning start to %p\n", current);
                return current;
            }

            current = current -> next;
        }

        //memory region with enough space in it was not found
        return NULL;
    }

    else if (strcmp(algo, "best_fit") == 0) 
    {
        /* best fit block */
        struct mem_block * block = NULL;
        struct mem_block * current = g_head;

        while (current != NULL)
        {
            /* free space of the current block */
            size_t free_space = (current -> size) - (current -> usage);

            /* user size to malloc */
            size_t requested = size + sizeof(struct mem_block);

            if (free_space >= requested && block == NULL) 
            {
                block = current;
            }
            else if (free_space >= requested && block != NULL)
            {
                size_t previously_found_block_free_space = block -> size - block -> usage;

                if(free_space < previously_found_block_free_space)
                {
                    block = current;
                }
            }

            current = current -> next;
        }
        return block;
    }

    else if (strcmp(algo, "worst_fit") == 0) 
    {

        /* worst fit block */
        struct mem_block * block = NULL;
        struct mem_block * current = g_head;

        while (current != NULL)
        {
            /* free space of the current block */
            size_t free_space = (current -> size) - (current -> usage);

            /* user size to malloc */
            size_t requested = size + sizeof(struct mem_block);

            if (free_space >= requested && block == NULL) 
            {
                block = current;
            }
            else if (free_space >= requested && block != NULL)
            {
                size_t previously_found_block_free_space = block -> size - block -> usage;

                if(free_space > previously_found_block_free_space) 
                {
                    block = current;
                }
            }
            current = current -> next;
        }
        return block;
    }

    //returns null here for syntactical reasons and if the algo isn't correctly formatted for whatever reason
    return NULL;
}

/**
 * malloc calls malloc_name to allocate memory via mmap
 * NOTE: an integer overflow would not be detected in malloc(nmemb * size);
 * @param size requested allocation size
 * @brief allocating memory from a given ammount of bytes given from size; returning
 * the needed space via malloc_name; it handles resizing over-allocated space, memory allocation of new memory, or returns NULL if certain sizes are. 
 */
void *malloc(size_t size)
{
    LOGP("Using my custom allocator instead of the system call\n");
    char * nm = "mem_block";
    LOGP("entering custom malloc\n");
    void * space = malloc_name(size, nm);
    LOG("malloc name returned %p\n", space);
    return space;
}

/**
 * Request new memory regions from the kernel via mmap
 * @param size requested allocation size
 * @param name name of the allocated block
 * @brief To perform the allocation, place a metadata struct at the start of
 * a free memory address and then return a pointer to the ‘data’ portion of the
 * memory shown in the first figure. 
 */
void *malloc_name(size_t size, char * name)
{
    pthread_mutex_lock(&thread_safety);
    LOG("Allocation request with size = %zu\n", size);
    // TODO: allocate memory. You'll first check if you can reuse an existing
    // block. If not, map a new memory region.
    /* malloc returns null when the size is zero */
    if (size == 0)
    {
        LOGP("size was 0\n");
        pthread_mutex_unlock(&thread_safety);
        return NULL;
    }

    if(size % 8 != 0) 
    {
        size += 8 - size % 8;
    }

    LOG("Adjusted size is %zu (if it was needed)\n", size);

    if(size + sizeof(struct mem_block) > M_MMAP_THRESHOLD) 
    {
        pthread_mutex_unlock(&thread_safety);
        return NULL;
    }
    LOG("what is size max %d\n", M_MMAP_THRESHOLD);

    // ... ask about here: casting to mem block from reuse
    struct mem_block * unused_memory;
    if ( (unused_memory = reuse(size)) != NULL)
    {
        if(unused_memory -> usage == 0)
        {
            unused_memory -> usage = size + sizeof(struct mem_block);
            unused_memory -> alloc_id = g_allocations++;
            strcpy(unused_memory -> name, name);
            pthread_mutex_unlock(&thread_safety);
            return unused_memory + 1;
        }
        LOGP("There is unused memory and the process has been locked\n");

        // TODO: finish the resize code without the free block and see how that code goes
        size_t org_size = unused_memory -> size;
        LOG("size of unused memory %zu\n", org_size);
        size_t org_usage = unused_memory -> usage;
        struct mem_block * next_block = unused_memory -> next;
        size_t needed_space = size + sizeof(struct mem_block);
        /* 
         * This line is for adding a free block after; however, because that can get
         * difficult when it comes to logic and checking, it may be better to just
         * keep things simple and just have the initial space and the new block
         * without creating a free_block
         */
        LOG("malloc -> reuse:unused_memory start %p to %p and usage is %zu\n", unused_memory, (void *)unused_memory + unused_memory -> size, unused_memory -> usage);

        /* code for splitting the memory up that needs to be freed that is extra should go here */
        struct mem_block * new_block = (struct mem_block *) ((void *)unused_memory + org_usage);
        new_block -> region_start = unused_memory -> region_start;
        new_block -> size = unused_memory -> size - unused_memory -> usage;
        LOG("free_space %zu\n", new_block -> size);
        new_block -> usage = needed_space;
        new_block -> alloc_id = g_allocations++;
        new_block -> next = next_block;
        strcpy(new_block -> name, name);
        LOG("malloc -> reuse new_block: %p-%p (%lu) '%s' %zu %zu %zu\n", new_block, (void *)new_block + new_block -> size, new_block -> alloc_id, 
        new_block -> name, new_block -> size, new_block -> usage, new_block -> usage - sizeof(struct mem_block));
        /* region_size of the new_block is undefined because it is a split block */

        /* 
         * updating the original block space so that it 
         * reflects the changes of having been split
         */
        unused_memory -> size = org_usage;
        unused_memory -> next = new_block;
        LOG("malloc -> reuse new_block: data end %p\n", (void *)new_block + new_block -> usage);
        pthread_mutex_unlock(&thread_safety);
        return new_block + 1;
    }

    /* seting up the page size, the actual size of the memory, and the number of pages the memory region should have */
    int page_size = getpagesize();
    size_t actual_size = size + sizeof(struct mem_block);
    size_t number_of_pages = actual_size / page_size;
    LOG("page size %d actual size %zu number of pages %zu\n", page_size, actual_size, number_of_pages);

    /* always allocating at least a page of memory no matter what ... ask about here */
    if (actual_size % page_size != 0)
    {
        number_of_pages++;
    }

    LOG("number of pages %zu\n", number_of_pages);

    size_t region_size = number_of_pages * page_size;
    LOG("region size %zu\n", region_size);

    /*
     * Create the memory block
     * MMAP_THRESHOLD When allocating blocks of memory larger than MMAP_THRESHOLD bytes, the glibc malloc()
     * implementation allocates the memory as a private anonymous mapping using mmap(2). Hence, the need
     * for using MAP_ANONYMOUS when calling mmap
     */
    struct mem_block * region = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    /* checking to see if mapping the address of memory had failed for some reason */
    if (region == MAP_FAILED)
    {
        LOGP("error mapping memory\n");
        perror("mmap");
        pthread_mutex_unlock(&thread_safety);
        return NULL;
    }
    LOGP("mapped memory\n");

    char * scribble = getenv("ALLOCATOR_SCRIBBLE");

    LOG("got %s for scribble\n", scribble);
    /* scribble with junk */
    if(scribble != NULL)
    {
        if (strcmp(scribble, "1") == 0)
        {    
            LOGP("memset fail?\n");
            memset((void *)region + sizeof(struct mem_block), 0xAA, region_size - sizeof(struct mem_block));
        }
        else memset((void *)region + sizeof(struct mem_block), 0xAA, region_size - sizeof(struct mem_block));
        /* clear instead; no need to scribble with 0xAA */
    }
    else
    {
        LOGP("memset fail?\n");
        memset((void *)region + sizeof(struct mem_block), 0xAA, region_size - sizeof(struct mem_block));
    }

    LOGP("memset worked\n");

    /* setting up the region mem_block structure */
    strcpy(region -> name, name);
    region -> alloc_id = g_allocations++;
    region -> size = region_size;
    region -> usage = actual_size;
    region -> region_start = region;
    region -> region_size = region_size;
    region -> next = NULL;
    LOG("malloc -> region: %p-%p (%lu) '%s' %zu %zu %zu\n", region, (void *)region + region -> size, region -> alloc_id, region -> name, region -> size,
    region -> usage, region -> usage - sizeof(struct mem_block));

    /* adding the memory allocation to the linked list */
    if (g_head == NULL)
    {
        //set the head of the linked list
        g_head = region;
    }
    else
    {
        //find the last region of memory and then
        struct mem_block * current = g_head;
        while (current -> next != NULL) current = current -> next;
        current -> next = region;
        LOG("current %p and the next %p\n", current, current -> next);
    }
    /* returning where the memory after the region struct starts */
    LOG("Data end %p\n", (void *)region + region -> usage);
    pthread_mutex_unlock(&thread_safety);
    return region + 1;
}

/**
 * The free() function frees the memory space pointed to by ptr, which
 * must have been returned by a previous call to malloc(), calloc(), or
 * realloc().
 * @param ptr memory block pointed to by ptr
 * @brief set usage to zero or unmap memory region
 */
void free(void *ptr)
{
    pthread_mutex_lock(&thread_safety);
    if (ptr == NULL) {
        /* Freeing a NULL pointer does nothing */
        pthread_mutex_unlock(&thread_safety);
        return;
    }

    /* free the block from the start */
    struct mem_block * freed_block = (struct mem_block *) ptr - 1;
    freed_block -> usage = 0;
    LOG("freed block %p\n", freed_block);
    memset(ptr, 0xAA, freed_block -> size - sizeof(struct mem_block));
    //blocks
    struct mem_block * current = g_head;
    struct mem_block * next = NULL;
    struct mem_block * last = NULL;

    //region beginnings and endings
    struct mem_block * current_first = NULL;

    if(freed_block -> region_start == g_head)
    {
        while(current != NULL)
        {
            if(current -> usage != 0) 
            {
                LOG("%p has a non zero usage\n", current);
                pthread_mutex_unlock(&thread_safety);
                return;
            }
            next = current -> next;
            if(next == NULL
            || next -> region_start != current -> region_start)
            {
                last = current;
                LOGP("looping is finished\n");
                break;
            }
            current = next;
        }
        g_head = (last == NULL)? NULL : last -> next;
        last -> next = NULL;
        current_first = freed_block -> region_start;
        int value = munmap(current_first, current_first -> region_size);
        if(value == -1) 
        {
            LOGP("munmapped has failed T~T\n");
            perror("munmap");
        }
        else
            LOGP("unmapped memory correctly ^o^\n");
        pthread_mutex_unlock(&thread_safety);
        return;
    }
    //blocks: also setting the blocks to initial postitions just in case there are any edge cases I may be forgetting somehow
    struct mem_block * previous = NULL;
    current = g_head;
    next = NULL;
    last = NULL;
    LOGP("region to free is not be g_head\n");

    //region beginnings and endings: also setting the regionss to initial postitions just in case there are any edge cases I may be forgetting somehow
    struct mem_block * previous_last = NULL;
    current_first = freed_block -> region_start;
    struct mem_block * next_first = NULL;
    while(current != NULL)
    {
        next = current -> next;
        //look for previous block
        if(next -> region_start != current_first && previous_last == NULL)
        {
            previous = current;
            current = next;
            LOG("previous %p and current %p\n", previous, current);
            continue;
        }

        //set previous block once
        if(next -> region_start == current_first && previous_last == NULL)
        {
            previous_last = current;
            current = next;
            LOG("previous_last is set %p\n", previous_last);
        }
        LOG("next is %p\n", next);

        //find the last block in current region
        if((current -> region_start == current_first && previous_last != NULL) || current -> region_size == current_first -> region_size)
        {
            if(current -> usage != 0) 
            {
                LOG("%p has a non zero usage\n", current);
                pthread_mutex_unlock(&thread_safety);
                return;
            }
            next = current -> next;
            if(next == NULL
            || next -> region_start != current -> region_start)
            {
                last = current;
                LOGP("looping is finished\n");
                break;
            }
            current = next;
            continue;
        }

    }
    LOGP("time to free a region\n");
    next_first = (last  == NULL)? NULL : last -> next;
    previous_last -> next = next_first;
    
    LOG("previous region last block %p and next region first block %p\n", previous_last, next_first);

    int value = munmap(current_first, current_first -> region_size);
    if(value == -1)
    {
        LOGP("munmapped has failed T~T\n");
        perror("munmap");
    }
    else
        LOGP("unmapped memory correctly ^o^\n");
    pthread_mutex_unlock(&thread_safety);
    return;

    /*
     * TODO: finish code for a previous_region, current_region, and next_region when going through a region
     * check to see if we need to free regions and fix the linked list
     * 
     * TODO: FINDING REGION TO UNMAP IF NEEDED
     * Always unmapping the current_region(∆) if it is needed
     * previous_region | current_region | next_region
     * p c n
     * - - - –> fixing linked list
     * n ∆ n –> g_head = NULL || next_region;                                  either of can be used because next_region will be NULL
     * n ∆ • –> g_head = next_region;
     * • ∆ n –> previous_region -> next = NULL || next_region;                 either of can be used because next_region will be NULL
     * • ∆ • –> previous_region -> next = next_region;
     * 
     * 
     * 
     * 
     */
}

/**
 * calloc will clear the memory and return NULL if overflow occurs, 
 * @param size requested allocation size
 * @param nmemb elements of size bytes each and returns a pointer to the allocated memory
 * @brief The calloc() function allocates memory for an array of nmemb elements
 * of size bytes each and returns a pointer to the allocated memory.
 */
void *calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0) return NULL;

    /* requested size */
    size_t max = nmemb * size;

    /* adjusted data size */
    if(max % 8 != 0) max += 8 - max % 8;

    /* header and the data */
    size_t adjusted_max = max + sizeof(struct mem_block);

    /* if there won't be enough for the full space due to overflow ...ask about here for overflow*/
    if (adjusted_max > SIZE_MAX) return NULL;

    // TODO: question about the where overflow is needed; it should only be here?
    // I may not need to do the three previous
    // lines of code but doing so may make sure the
    // amount of memory aligned and adjusted with
    // the struct won't cause an overflow issue
    // Answer: in terms of known implementation, overflow happens here where as
    // in malloc it an integer overflow would not be detected
    // TODO: hmm, what does calloc do?
    void * pointer = malloc(max);

    if(!pointer)
    {
        errno = ENOMEM;
        return NULL;
    }

    memset(pointer, 0, max);
    //cleared data yaaaaay!

    return pointer;
}

/**
 * The realloc() function changes the size of the memory block pointed
 * to by ptr to size bytes.  The contents will be unchanged in the range
 * from the start of the region up to the minimum of the old and new sizes
 * @param size requested size
 * @param ptr memory block pointed to by ptr
 * @brief memory block of memory sectioned out into chunks, new space within a region, or 
 * NULL if there were any issues raised
 */
void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        /* If the pointer is NULL, then we simply malloc a new block */
        return malloc(size);
    }

    if (size == 0) {
        /* Realloc to 0 is often the same as freeing the memory block... But the
         * C standard doesn't require this. We will free the block and return
         * NULL here. */
        free(ptr);
        return NULL;
    }

    if(size % 8 != 0) size += 8 - size % 8;

    // TODO: reallocation logic
    struct mem_block * block = (struct mem_block *) ptr - 1;
    size_t original_requested_space = (block -> usage);
    size_t original_size = (block -> size);
    size_t needed_allocation_space = size + sizeof(struct mem_block);

    // TODO: check for the usage if it can be resized in place ... ask about here
    if (original_requested_space < original_size && original_requested_space >= needed_allocation_space)
    {
       block -> usage = needed_allocation_space;
       LOG("usage of realloc block %zu\n", block -> usage);
       return block + 1;
    }

    /* if there isn't enough space for the block to be resized, we just malloc space; malloc also handles reusing blocks so that isn't needed to be done here */
    LOGP("All memory is used in linked list so new space will be allocated\n");
    void * new_ptr;
    new_ptr = malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, original_size);
    free(ptr);
    return new_ptr;

}
//End of file
