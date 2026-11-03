# Operating Systems — Assignment 10 (xv6)

**Student Name**: Nishchay  
**Roll Number**: 2401mc22  

This repository contains the complete kernel modifications, user test programs, terminal output screenshots, and report for **Operating Systems Assignment 10** on **MIT xv6 (x86)**.

---

## Overview of Implemented Solutions

### Question 1 — Memory Management: Tracking Physical Memory Allocation (`memstats` & `memtest.c`)
- **Physical Page Counters in [`kalloc.c`](kalloc.c)**:
  - Extended `struct kmem` with `total_allocated`, `total_freed`, `free_pages`, and `initializing`.
  - `kalloc()` increments `kmem.total_allocated` and decrements `kmem.free_pages` on every successful page allocation.
  - `kfree()` increments `kmem.free_pages` and, after boot-time `freerange()` initialization (`!kmem.initializing`), increments `kmem.total_freed`.
- **System Call `int memstats(void)` (`SYS_memstats = 22`)**:
  - Displays `Total allocated pages`, `Total freed pages`, `Currently allocated pages`, and `Available physical pages`, and returns the currently allocated page count.
- **User Program [`memtest.c`](memtest.c)**:
  - Displays baseline memory statistics, allocates 5 pages (`20480` bytes) and then 3 additional pages (`12288` bytes) using `sbrk()`, displays the updated statistics (`+5` and `+8` currently allocated pages), releases all 8 pages via `sbrk(-8 * 4096)`, and verifies that `Total freed pages` increases by 8 and available physical memory returns to its initial value.

### Question 2 — Virtual Memory: Virtual-to-Physical Address Translation (`vtop` & `vttest.c`)
- **System Call `int vtop(int virtual_address)` (`SYS_vtop = 23`)**:
  - Implemented in [`vm.c`](vm.c) using `walkpgdir(curproc->pgdir, (const void*)va, 0)`.
  - Checks whether the virtual address lies in user space (`< KERNBASE`) and whether its page-table entry is valid and present (`PTE_P`).
  - Extracts the 20-bit physical frame base address (`PTE_ADDR(*pte)`) and combines it with the 12-bit page offset (`va & 0xFFF`) to return the complete physical address (`frame_addr | offset`), or `-1` if unmapped/invalid.
- **User Program [`vttest.c`](vttest.c)**:
  - Allocates 2 heap pages via `sbrk(2 * 4096)` and translates code (`main`), stack (`&stack_var`), page-aligned heap bases (`0x3000`, `0x4000`), and interior heap addresses (`0x3123`, `0x4abc`), demonstrating exact offset preservation.
  - Tests unmapped user (`0x50000`) and kernel (`0x80100000`) virtual addresses, verifying that `vtop()` returns `-1`.

### Question 3 — Paging: Page Table Management and Page Protection (`pageinfo`, `setreadonly` & `pagetest.c`)
- **System Call `int pageinfo(int virtual_address)` (`SYS_pageinfo = 24`)**:
  - Uses `walkpgdir()` in [`vm.c`](vm.c) to check whether `virtual_address` is mapped (`PTE_P`), and displays its mapping status, physical frame number (`PFN = PTE_ADDR(*pte) >> 12`), physical frame address, and page-table flags (`P`, `W`, `U`).
- **System Call `int setreadonly(int virtual_address)` (`SYS_setreadonly = 25`)**:
  - Clears the write permission bit (`*pte &= ~PTE_W`) in the page-table entry for `virtual_address` and flushes the x86 TLB by reloading `CR3` (`lcr3(V2P(curproc->pgdir))`).
- **User Program [`pagetest.c`](pagetest.c)**:
  - Allocates a page via `sbrk(4096)`, writes `'A'`, displays its PTE via `pageinfo()` (`P=1 W=1 U=1`), calls `setreadonly()`, verifies via `pageinfo()` that `W` is now `0` (`P=1 W=0 U=1`), confirms reading still works, and attempts a write (`page[0] = 'B'`) to trigger a hardware Page Fault (`trap 14 err 7 -- kill proc`).

### Question 4 — Page Replacement Algorithms: FIFO, LRU and Optimal ([`pagereplace.c`](pagereplace.c))
- **User Program [`pagereplace.c`](pagereplace.c)**:
  - Implements **FIFO**, **LRU**, and **Optimal** page replacement algorithms and displays the frame state and `Page HIT` / `Page FAULT` for every reference.
  - Tests all three algorithms on the 20-page reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` with **3 frames** (**15** faults for FIFO, **12** for LRU, **9** for Optimal).
  - Evaluates all three algorithms across **3, 4, and 5 frames** and prints a full comparison table of page faults, page hits, hit ratio, and page fault ratio.

---

## Modified & Added Files

| File | Description |
| :--- | :--- |
| [`kalloc.c`](kalloc.c) | Added allocation/deallocation counters in `kalloc()` and `kfree()`; implemented `memstats()`. |
| [`vm.c`](vm.c) | Exposed `walkpgdir()`; implemented `vtop()`, `pageinfo()`, and `setreadonly()`. |
| [`sysproc.c`](sysproc.c) | Added `sys_memstats()`, `sys_vtop()`, `sys_pageinfo()`, and `sys_setreadonly()`. |
| [`syscall.h`](syscall.h), [`syscall.c`](syscall.c), [`defs.h`](defs.h), [`user.h`](user.h), [`usys.S`](usys.S) | Added system call numbers (`22`–`25`), kernel prototypes, and user-space stubs. |
| [`Makefile`](Makefile) | Added `_memtest`, `_vttest`, `_pagetest`, and `_pagereplace` to `UPROGS` and `EXTRA`. |
| [`memtest.c`](memtest.c) | User test program for Question 1 (`memstats` & `sbrk`). |
| [`vttest.c`](vttest.c) | User test program for Question 2 (`vtop` virtual-to-physical translation). |
| [`pagetest.c`](pagetest.c) | User test program for Question 3 (`pageinfo`, `setreadonly`, and read-only page fault). |
| [`pagereplace.c`](pagereplace.c) | User program for Question 4 (FIFO, LRU, and Optimal page replacement). |
| [`REPORT.md`](REPORT.md) | Detailed report explaining implementations, modifications, algorithms, and output observations. |
| [`screenshots/`](screenshots/) | Terminal output screenshots (`q1_memtest.png`, `q2_vttest.png`, `q3_pagetest.png`, `q4_pagereplace.png`). |

---

## How to Build and Run

```bash
# Clean and build xv6
make clean
make

# Launch xv6 in headless QEMU
make qemu-nox

# Inside the xv6 shell ($):
$ memtest
$ vttest
$ pagetest
$ pagereplace
```
