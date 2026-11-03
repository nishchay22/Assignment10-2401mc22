// ============================================================================
// Operating Systems Assignment 10 — Question 3(c): pagetest.c
//
// User test program for page table inspection and page protection:
//   1. Allocates a memory page via sbrk(4096) and writes initial data.
//   2. Displays its page-table information (PFN and flags P, W, U) via pageinfo().
//   3. Removes write permission (PTE_W) via setreadonly() and verifies via
//      pageinfo() that W changed from 1 to 0.
//   4. Verifies that reading from the read-only page succeeds, then attempts
//      to write to the protected page to demonstrate that a hardware page
//      fault (trap 14 / T_PGFLT) occurs.
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  char *page;

  printf(1, "============================================================\n");
  printf(1, "Question 3: Page Table Management & Page Protection (pagetest)\n");
  printf(1, "============================================================\n\n");

  printf(1, "[1] Allocating 1 page (4096 bytes) using sbrk(4096)...\n");
  page = sbrk(PGSIZE);
  if(page == (char*)-1){
    printf(1, "pagetest: sbrk failed\n");
    exit();
  }
  page[0] = 'A';
  printf(1, "    Allocated page at VA 0x%x; initial write page[0] = '%c' succeeded.\n",
         (uint)page, page[0]);

  printf(1, "\n[2] Inspecting page-table information before protection change:\n");
  pageinfo((int)page);
  printf(1, "    Inspecting an unmapped address (0x60000) for comparison:\n");
  pageinfo(0x60000);

  printf(1, "\n[3] Calling setreadonly(0x%x) to remove write permission (PTE_W)...\n",
         (uint)page);
  if(setreadonly((int)page) < 0){
    printf(1, "pagetest: setreadonly failed\n");
    exit();
  }

  printf(1, "\n[4] Inspecting page-table information after setreadonly():\n");
  pageinfo((int)page);

  printf(1, "\n[5] Reading from read-only page at VA 0x%x: page[0] = '%c' (Read OK)\n",
         (uint)page, page[0]);

  printf(1, "\n[6] Attempting to write to read-only page at VA 0x%x (expecting trap 14 page fault)...\n",
         (uint)page);
  // This write violates PTE_W=0 in user mode and triggers Trap 14 (T_PGFLT)
  page[0] = 'B';

  // Should never be reached because the kernel kills the process on page fault
  printf(1, "ERROR: Write to read-only page did not fault!\n");
  exit();
}
