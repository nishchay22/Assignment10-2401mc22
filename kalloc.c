// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  // Assignment 10 (Question 1a): Counters to track physical page allocation and deallocation
  uint total_allocated;   // Total number of pages allocated by kalloc()
  uint total_freed;       // Total number of pages freed by kfree() (post-initialization)
  uint free_pages;        // Current number of available physical pages on freelist
  int initializing;       // Set to 1 while freerange() populates initial free pool
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  kmem.total_allocated = 0;
  kmem.total_freed = 0;
  kmem.free_pages = 0;
  kmem.initializing = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  // Assignment 10 (Question 1a): Mark boot-time free list population so initial
  // physical memory discovery is counted as available pages, not deallocations.
  kmem.initializing = 1;
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
  kmem.initializing = 0;
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  // Assignment 10 (Question 1a): Increment available pages and, if not in
  // boot-time freerange() initialization, increment total freed pages counter.
  kmem.free_pages++;
  if(!kmem.initializing)
    kmem.total_freed++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    // Assignment 10 (Question 1a): Increment total allocated pages and
    // decrement available physical pages counter.
    kmem.total_allocated++;
    if(kmem.free_pages > 0)
      kmem.free_pages--;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

// Assignment 10 (Question 1b): System call helper to display physical memory statistics:
// total allocated pages, total freed pages, currently allocated pages, and
// available physical memory pages.
int
memstats(void)
{
  struct run *r;
  uint total_alloc, total_free, curr_alloc, avail_pages;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  avail_pages = 0;
  for(r = kmem.freelist; r != 0; r = r->next)
    avail_pages++;

  total_alloc = kmem.total_allocated;
  total_free  = kmem.total_freed;
  curr_alloc  = total_alloc - total_free;

  if(kmem.use_lock)
    release(&kmem.lock);

  cprintf("--- Physical Memory Statistics (memstats) ---\n");
  cprintf("  Total allocated pages     : %d\n", total_alloc);
  cprintf("  Total freed pages         : %d\n", total_free);
  cprintf("  Currently allocated pages : %d\n", curr_alloc);
  cprintf("  Available physical pages  : %d (%d KB)\n", avail_pages, avail_pages * 4);
  cprintf("---------------------------------------------\n");

  return (int)curr_alloc;
}
