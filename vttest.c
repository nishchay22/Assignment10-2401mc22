// ============================================================================
// Operating Systems Assignment 10 — Question 2(c): vttest.c
//
// User test program for virtual-to-physical address translation (vtop):
//   1. Allocates heap memory using sbrk().
//   2. Passes code, stack, and multiple heap virtual addresses (both page-
//      aligned and with byte offsets) to vtop() and prints their corresponding
//      physical addresses.
//   3. Passes invalid/unmapped virtual addresses to vtop() and demonstrates
//      that vtop() detects unmapped PTEs and returns -1.
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

static void
translate_and_print(char *label, uint va)
{
  int pa;

  printf(1, "\n  Test: %s\n", label);
  pa = vtop((int)va);
  if(pa < 0){
    printf(1, "  Result: VA 0x%x -> INVALID / UNMAPPED (vtop returned %d)\n", va, pa);
  } else {
    printf(1, "  Result: VA 0x%x -> PA 0x%x (Frame: 0x%x, Offset: 0x%x)\n",
           va, (uint)pa, (uint)pa & ~0xFFF, (uint)pa & 0xFFF);
  }
}

int
main(int argc, char *argv[])
{
  int stack_var = 42;
  char *heap;

  printf(1, "============================================================\n");
  printf(1, "Question 2: Virtual-to-Physical Address Translation (vttest)\n");
  printf(1, "============================================================\n");

  printf(1, "\n[1] Allocating 2 heap pages (8192 bytes) via sbrk(2 * 4096)...\n");
  heap = sbrk(2 * PGSIZE);
  if(heap == (char*)-1){
    printf(1, "vttest: sbrk failed\n");
    exit();
  }
  // Write to both allocated pages
  heap[0] = 'H';
  heap[0x123] = 'E';
  heap[PGSIZE] = 'A';
  heap[PGSIZE + 0xABC] = 'P';
  printf(1, "    Allocated 2 pages starting at VA 0x%x\n", (uint)heap);

  printf(1, "\n[2] Translating valid mapped virtual addresses using vtop():\n");
  translate_and_print("Code segment (main function)", (uint)main);
  translate_and_print("User stack variable (&stack_var)", (uint)&stack_var);
  translate_and_print("Heap Page 0 base (page-aligned, offset 0x000)", (uint)heap);
  translate_and_print("Heap Page 0 + 0x123 (interior offset 0x123)", (uint)(heap + 0x123));
  translate_and_print("Heap Page 1 base (page-aligned, offset 0x000)", (uint)(heap + PGSIZE));
  translate_and_print("Heap Page 1 + 0xABC (interior offset 0xABC)", (uint)(heap + PGSIZE + 0xABC));

  printf(1, "\n[3] Translating invalid / unmapped virtual addresses using vtop():\n");
  translate_and_print("Unmapped user VA above process size (0x50000)", 0x50000);
  translate_and_print("Kernel virtual address (0x80100000)", 0x80100000);

  sbrk(-2 * PGSIZE);
  printf(1, "\n============================================================\n");
  exit();
}
