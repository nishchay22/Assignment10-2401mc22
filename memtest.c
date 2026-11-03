// ============================================================================
// Operating Systems Assignment 10 — Question 1(c): memtest.c
//
// User test program for physical memory allocation tracking:
//   1. Displays baseline physical memory statistics via memstats().
//   2. Allocates physical pages using sbrk() and displays updated statistics.
//   3. Releases the allocated memory via negative sbrk() and verifies that
//      total freed pages and available physical pages update accurately.
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  char *p1, *p2;
  int i;
  int c0, c1, c2, c3;

  printf(1, "============================================================\n");
  printf(1, "Question 1: Physical Memory Allocation Tracking (memtest)\n");
  printf(1, "============================================================\n\n");

  printf(1, "[1] Initial memory statistics before sbrk() allocation:\n");
  c0 = memstats();

  printf(1, "\n[2] Allocating 5 pages (20480 bytes) using sbrk(5 * 4096)...\n");
  p1 = sbrk(5 * PGSIZE);
  if(p1 == (char*)-1){
    printf(1, "memtest: sbrk failed\n");
    exit();
  }
  // Touch each allocated page to verify physical mapping
  for(i = 0; i < 5; i++)
    p1[i * PGSIZE] = (char)('A' + i);

  printf(1, "    Base VA of 5-page block: 0x%x\n", (uint)p1);
  printf(1, "    Memory statistics after allocating 5 pages:\n");
  c1 = memstats();
  printf(1, "    -> Delta currently allocated pages: +%d (Expected: +5)\n", c1 - c0);

  printf(1, "\n[3] Allocating 3 additional pages (12288 bytes) using sbrk(3 * 4096)...\n");
  p2 = sbrk(3 * PGSIZE);
  if(p2 == (char*)-1){
    printf(1, "memtest: second sbrk failed\n");
    exit();
  }
  for(i = 0; i < 3; i++)
    p2[i * PGSIZE] = (char)('X' + i);

  printf(1, "    Memory statistics after allocating 3 more pages (8 total):\n");
  c2 = memstats();
  printf(1, "    -> Delta currently allocated pages from start: +%d (Expected: +8)\n", c2 - c0);

  printf(1, "\n[4] Releasing all 8 allocated pages (32768 bytes) using sbrk(-8 * 4096)...\n");
  if(sbrk(-8 * PGSIZE) == (char*)-1){
    printf(1, "memtest: sbrk deallocation failed\n");
    exit();
  }
  printf(1, "    Updated memory statistics after releasing 8 pages:\n");
  c3 = memstats();
  printf(1, "    -> Net change from initial currently allocated pages: %d (Expected: 0)\n", c3 - c0);
  printf(1, "============================================================\n");

  exit();
}
