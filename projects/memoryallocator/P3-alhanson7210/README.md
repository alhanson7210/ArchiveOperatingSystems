# Project 3: Memory Allocator

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-3.html 

*Writer: Alex Hanson*

University of San Francisco

CS 326 Operating Systems

## About This Project:

_CREATING A MEMORY ALLOCATOR FOR MEMORY MANAGEMENT FOR AN OPERATING SYSTEM BY USING 
OR CREATING THE NECESSARY SYSTEM CALLS TO ALLOCATE AND FREE MEMORY ACCORDINGLY_

*Memory Allocator*
> The program allocates a page of space or reuses a given block of pre-allocated 
> memory that the user will use based of a given request size which will also have 
> the mem_block attached before that allocated memory request

*Managing Memory*
> In order to keep track of memory when a user wants to free or malloc memory(et al)
> is done by have a linked list of memory that memory blocks get added or deleted from.
> First fit: looks for the first block size that fits the memory request
> Best fit: looks for the minimum block size that fits the memory request
> Worst fit: looks for the maximum block size that fits the memory request  

*Allocating Memory*
> MMAP is used to allocate memory more flexibly than sbrk or brk, but it also requires
> more parameters to be filled. Overflow is checked in calloc, M_MMAP_THRESHOLD is
> is checked for in malloc, and ENOMEM can additional handled in realloc, and lastly
> you can check for SIZE_MAX if a memory request is too large as well 

*Freeing Memory*
> The way freeing is managed is by checking a block of memory to see if it is g_head or not
> If it is, this requires going through the linked list and simultaneously checking the region
> for all its blocks having a usage of 0. If at any point the usage doesn't equal zero, simply
> return; otherwise, the region actual has to be unmmapped.
> If it's not g_head, this requires finding the previous regions last block and the the next
> regions first block. If at any point the usage doesn't equal zero, simply
> return; otherwise, the region actual has to be unmmapped as well.
> Depending on the case, g_head or some other randomly placed block is unmapped, and the linked
> list is fixed in place

*Reallocating Memory*
> If the block can be resized in place, we can simply change the usage; otherwise we need to
> malloc space for a new block, copy the old one to the new, free the old one, and return the new

*Basic Functionality Overview*
```
  1. Simple allocator (one block per region)
  2. print_memory() function
  3. Verified(correct) linked list sequence and visualization
  4. Scribbling
  5. Free space management algorithms
```

*Extra Features*
> Named Blocks: to help with debugging, you can optionally provide a name for each allocation. 
> These names will be shown when state information is printed.
> Memory State Information: your allocator should be able to print out the current state of 
> the regions and blocks with the print_memory() function. See the format below:
> File Logging: You should also be able to write the same contents printed by print_memory to a file.
> Scribbling: free() leaves old values in memory without cleaning them up. When the ALLOCATOR_SCRIBBLE 
> environment variable is set to 1, you will fill any new allocation with 0xAA (10101010 in binary). 
> This means that if a program assumes memory allocated by malloc is zeroed out, it will crash

## Visualizations:
```
-- Current Memory State --
[REGION] 0x7fdb7f7cc000-0x7fdb7f7cd000 4096
[BLOCK]  0x7fdb7f7cc000-0x7fdb7f7cc248 (0) 'Test Allocation: 0' 584 584 504
[BLOCK]  0x7fdb7f7cc248-0x7fdb7f7cc4f0 (5) 'Test Allocation: 5' 680 680 600
[BLOCK]  0x7fdb7f7cc4f0-0x7fdb7f7cc5d8 (6) 'Test Allocation: 6' 232 232 152
[BLOCK]  0x7fdb7f7cc5d8-0x7fdb7f7cc680 (7) 'Test Allocation: 7' 168 136 56
[BLOCK]  0x7fdb7f7cc680-0x7fdb7f7cc7d0 (2) 'Test Allocation: 2' 336 336 256
[BLOCK]  0x7fdb7f7cc7d0-0x7fdb7f7cc948 (3) 'Test Allocation: 3' 376 0 0
[BLOCK]  0x7fdb7f7cc948-0x7fdb7f7cd000 (4) 'Test Allocation: 4' 1720 584 504
[REGION] 0x7fdb7f7a1000-0x7fdb7f7a3000 8192
[BLOCK]  0x7fdb7f7a1000-0x7fdb7f7a3000 (8) 'mem_block' 8192 4176 4096
```

## Program Options:
```
LD_PRELOAD=$(pwd)/allocator.so command

allocation.h
/* -- Helper functions -- */
void print_memory(void);
void write_memory(FILE * fp);
void *reuse(size_t size);
void *malloc_name(size_t size, char * name);

/* -- C Memory API functions -- */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
```

## Data Structures and Globals:
```
/**
 * Defines metadata structure for both memory 'regions' and 'blocks.' This
 * structure is prefixed before each allocation's data area.
 */
struct mem_block {
    /**
     * Each allocation is given a unique ID number. If an allocation is split in
     * two, then the resulting new block will be given a new ID.
     */
    unsigned long alloc_id;

    /**
     * The name of this memory block. If the user doesn't specify a name for the
     * block, it should be auto-generated based on the allocation ID.
     */
    char name[32];

    /** Size of the memory region */
    size_t size;

    /** Space used; if usage == 0, then the block has been freed. */
    size_t usage;

    /**
     * Pointer to the start of the mapped memory region. This simplifies the
     * process of finding where memory mappings begin.
     */
    struct mem_block *region_start;

    /**
     * If this block is the beginning of a mapped memory region, the region_size
     * member indicates the size of the mapping. In subsequent (split) blocks,
     * this is undefined.
     */
    size_t region_size;

    /** Next block in the chain */
    struct mem_block *next;
};


static struct mem_block *g_head = NULL; /*!< Start (head) of our linked list */
static unsigned long g_allocations = 0; /*!< Allocation counter */
```

To compile and use the allocator:

```bash
make
LD_PRELOAD=$(pwd)/allocator.so ls /
```

(in this example, the command `ls /` is run with the custom memory allocator instead of the default).

```
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ls /
bin  boot  dev  etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  secret  srv  sys  tmp  usr  var
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:
```
[alhanson@alhanson7210 P3-alhanson7210]$ make test
./tests/run_tests
Building test programs
make[1]: Entering directory '/home/alhanson/projects/prj3/P3-alhanson7210/tests/progs'
make[1]: Nothing to be done for 'all'.
make[1]: Leaving directory '/home/alhanson/projects/prj3/P3-alhanson7210/tests/progs'
Running Tests: (v9)
 * 01 ls                   [1 pts]  [  OK  ]
 * 02 Free                 [1 pts]  [  OK  ]
 * 03 Basic First Fit      [1 pts]  [  OK  ]
 * 04 Basic Best Fit       [1 pts]  [  OK  ]
 * 05 Basic Worst Fit      [1 pts]  [  OK  ]
 * 06 First Fit            [1 pts]  [  OK  ]
 * 07 Best Fit             [1 pts]  [  OK  ]
 * 08 Worst Fit            [1 pts]  [  OK  ]
 * 09 Scribbling           [1 pts]  [  OK  ]
 * 10 Thread Safety        [1 pts]  [ FAIL ]
 * 11 print_memory()       [1 pts]  [  OK  ]
 * 12 write_memory()       [1 pts]  [  OK  ]
 * 13 Unix Utilities       [1 pts]  [ FAIL ]
 * 15 Static Analysis      [1 pts]  [  OK  ]
 * 16 Documentation        [1 pts]  [  OK  ]
 * 17 Thanks for Trying    [1 pts]  [  OK  ]
Execution complete. [14/16 pts] (87.5%)
```

## Program Output
```
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ls -lrt
total 5332
-rw-r--r-- 1 alhanson alhanson     642 Nov  5 13:58 README.md
-rw-r--r-- 1 alhanson alhanson    2135 Nov  5 13:58 debug.h
-rw-r--r-- 1 alhanson alhanson    2053 Nov 20 02:53 allocator.h
drwxr-xr-x 4 alhanson alhanson    4096 Nov 20 20:21 tests
-rw-r--r-- 1 alhanson alhanson   23688 Nov 20 22:06 allocator.c
-rw-r--r-- 1 alhanson alhanson     552 Nov 20 22:11 Makefile
-rwxr-xr-x 1 alhanson alhanson   24432 Nov 20 22:11 allocator.so
drwxr-xr-x 5 alhanson alhanson      42 Nov 20 22:11 docs
-rw-r--r-- 1 alhanson alhanson     595 Nov 20 22:11 test-memory-output-file.txt
-rw-r--r-- 1 alhanson alhanson 3062864 Nov 20 22:11 allocator.h.gch
-rw-r--r-- 1 alhanson alhanson 2258224 Nov 20 22:11 debug.h.gch
-rwxr-xr-x 1 alhanson alhanson   29592 Nov 20 22:11 a.out
-rw-r--r-- 1 alhanson alhanson   25461 Nov 20 22:11 test-output.md
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so free
              total        used        free      shared  buff/cache   available
Mem:        1008272       77156      807652         400      123464      797800
Swap:             0           0           0
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so getconf PAGE_SIZE
4096
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so echo "code works, what!!!!!!!!!"
LD_PRELOAD=./allocator.so echo "code works, whatLD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZE!"
code works, whatLD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZELD_PRELOAD=./allocator.so getconf PAGE_SIZE!
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ls /
bin  boot  dev  etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  secret  srv  sys  tmp  usr  var
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ps
  PID TTY          TIME CMD
 2673 pts/0    00:00:01 bash
32137 pts/0    00:00:00 ps
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so touch power_move.txt
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ls
allocator.c  allocator.h.gch  a.out    debug.h.gch  Makefile        README.md                    test-output.md
allocator.h  allocator.so     debug.h  docs         power_move.txt  test-memory-output-file.txt  tests
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so rm power_move.txt
[alhanson@alhanson7210 P3-alhanson7210]$ LD_PRELOAD=./allocator.so ls
allocator.c  allocator.h  allocator.h.gch  allocator.so  a.out  debug.h  debug.h.gch  docs  Makefile  README.md  test-memory-output-file.txt  test-output.md  tests
[alhanson@alhanson7210 P3-alhanson7210]$
```

## Running one or multiple cases:
```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
