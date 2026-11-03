// ============================================================================
// Operating Systems Assignment 10 — Question 4: pagereplace.c
//
// Implements and compares three page replacement algorithms in xv6:
//   1. FIFO (First-In First-Out)
//   2. LRU  (Least Recently Used)
//   3. Optimal Page Replacement (Belady's Optimal Algorithm)
//
// Tasks:
//   a) For every page reference, displays the current frames and indicates
//      whether it results in a page HIT or page FAULT.
//   b) Tests all three algorithms on the 20-page reference string:
//        7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
//      with 3 frames and displays total page faults for each algorithm.
//   c) Runs all three algorithms with 3, 4, and 5 frames on the same reference
//      string, prints a comparison table showing Page Faults, Page Hits,
//      Hit Ratio, and Page Fault Ratio, and explains which algorithm performs
//      best.
// ============================================================================

#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_FRAMES 16
#define MAX_REFS   128

static int default_refs[20] = {
  7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
  0, 3, 2, 1, 2, 0, 1, 7, 0, 1
};

// Search for page in frames[0..nframes-1]; returns index or -1 if not present
static int
find_page(int *frames, int nframes, int page)
{
  int i;
  for(i = 0; i < nframes; i++){
    if(frames[i] == page)
      return i;
  }
  return -1;
}

// Print current frame contents
static void
print_frames(int *frames, int nframes)
{
  int i;
  printf(1, "[");
  for(i = 0; i < nframes; i++){
    if(frames[i] == -1)
      printf(1, " -");
    else
      printf(1, " %d", frames[i]);
  }
  printf(1, " ]");
}

// Format and print ratio as "count/total (XX.X%)" without floating point
static void
print_ratio(int count, int total)
{
  int pct_whole = (count * 100) / total;
  int pct_frac  = ((count * 1000) / total) % 10;

  if(pct_whole < 10)
    printf(1, " %d.%d%%", pct_whole, pct_frac);
  else
    printf(1, "%d.%d%%", pct_whole, pct_frac);
}

// ----------------------------------------------------------------------------
// 1. FIFO (First-In First-Out) Page Replacement
// ----------------------------------------------------------------------------
static int
run_fifo(int *refs, int nrefs, int nframes, int verbose)
{
  int frames[MAX_FRAMES];
  int i, fifo_ptr = 0, faults = 0, hits = 0;

  for(i = 0; i < nframes; i++)
    frames[i] = -1;

  if(verbose)
    printf(1, "\n--- [1] FIFO (First-In First-Out) Page Replacement (%d Frames) ---\n", nframes);

  for(i = 0; i < nrefs; i++){
    int page = refs[i];
    int hit  = (find_page(frames, nframes, page) >= 0);

    if(hit){
      hits++;
    } else {
      frames[fifo_ptr] = page;
      fifo_ptr = (fifo_ptr + 1) % nframes;
      faults++;
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  Step  %d | Ref: %d -> Frames: ", i + 1, page);
      else
        printf(1, "  Step %d | Ref: %d -> Frames: ", i + 1, page);
      print_frames(frames, nframes);
      printf(1, " | %s\n", hit ? "Page HIT" : "Page FAULT");
    }
  }

  if(verbose){
    printf(1, "  >> FIFO Total Page Faults (%d frames): %d | Page Hits: %d\n",
           nframes, faults, hits);
  }
  return faults;
}

// ----------------------------------------------------------------------------
// 2. LRU (Least Recently Used) Page Replacement
// ----------------------------------------------------------------------------
static int
run_lru(int *refs, int nrefs, int nframes, int verbose)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int i, j, faults = 0, hits = 0;

  for(i = 0; i < nframes; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  if(verbose)
    printf(1, "\n--- [2] LRU (Least Recently Used) Page Replacement (%d Frames) ---\n", nframes);

  for(i = 0; i < nrefs; i++){
    int page = refs[i];
    int idx  = find_page(frames, nframes, page);

    if(idx >= 0){
      hits++;
      last_used[idx] = i;
    } else {
      int victim = 0;
      for(j = 0; j < nframes; j++){
        if(frames[j] == -1){
          victim = j;
          break;
        }
        if(last_used[j] < last_used[victim])
          victim = j;
      }
      frames[victim] = page;
      last_used[victim] = i;
      faults++;
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  Step  %d | Ref: %d -> Frames: ", i + 1, page);
      else
        printf(1, "  Step %d | Ref: %d -> Frames: ", i + 1, page);
      print_frames(frames, nframes);
      printf(1, " | %s\n", (idx >= 0) ? "Page HIT" : "Page FAULT");
    }
  }

  if(verbose){
    printf(1, "  >> LRU Total Page Faults  (%d frames): %d | Page Hits: %d\n",
           nframes, faults, hits);
  }
  return faults;
}

// ----------------------------------------------------------------------------
// 3. Optimal Page Replacement (replaces page used farthest in the future)
// ----------------------------------------------------------------------------
static int
run_optimal(int *refs, int nrefs, int nframes, int verbose)
{
  int frames[MAX_FRAMES];
  int i, j, k, faults = 0, hits = 0;

  for(i = 0; i < nframes; i++)
    frames[i] = -1;

  if(verbose)
    printf(1, "\n--- [3] Optimal Page Replacement (%d Frames) ---\n", nframes);

  for(i = 0; i < nrefs; i++){
    int page = refs[i];
    int idx  = find_page(frames, nframes, page);

    if(idx >= 0){
      hits++;
    } else {
      int victim = -1;
      int farthest = -1;

      for(j = 0; j < nframes; j++){
        if(frames[j] == -1){
          victim = j;
          break;
        }
        // Find next future reference of frames[j]
        int next_use = nrefs + 1;
        for(k = i + 1; k < nrefs; k++){
          if(refs[k] == frames[j]){
            next_use = k;
            break;
          }
        }
        if(next_use > farthest){
          farthest = next_use;
          victim = j;
        }
      }
      frames[victim] = page;
      faults++;
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  Step  %d | Ref: %d -> Frames: ", i + 1, page);
      else
        printf(1, "  Step %d | Ref: %d -> Frames: ", i + 1, page);
      print_frames(frames, nframes);
      printf(1, " | %s\n", (idx >= 0) ? "Page HIT" : "Page FAULT");
    }
  }

  if(verbose){
    printf(1, "  >> Optimal Total Page Faults (%d frames): %d | Page Hits: %d\n",
           nframes, faults, hits);
  }
  return faults;
}

static void
print_row(char *alg, int frames, int faults, int nrefs)
{
  int hits = nrefs - faults;
  printf(1, "  %d    | %s |     %d     |    %d    |  ",
         frames, alg, faults, hits);
  print_ratio(hits, nrefs);
  printf(1, "    |   ");
  print_ratio(faults, nrefs);
  printf(1, "\n");
}

static void
print_comparison_table(int *refs, int nrefs)
{
  int f;

  printf(1, "\n====================================================================\n");
  printf(1, "Task (c): Comparison Table for 3, 4, and 5 Frames (Total Refs = %d)\n", nrefs);
  printf(1, "====================================================================\n");
  printf(1, "Frames | Algorithm | PageFaults | PageHits | Hit Ratio | Fault Ratio\n");
  printf(1, "-------+-----------+------------+----------+-----------+------------\n");

  for(f = 3; f <= 5; f++){
    int fifo_f = run_fifo(refs, nrefs, f, 0);
    int lru_f  = run_lru(refs, nrefs, f, 0);
    int opt_f  = run_optimal(refs, nrefs, f, 0);

    print_row("FIFO     ", f, fifo_f, nrefs);
    print_row("LRU      ", f, lru_f,  nrefs);
    print_row("Optimal  ", f, opt_f,  nrefs);
    if(f < 5)
      printf(1, "-------+-----------+------------+----------+-----------+------------\n");
  }
  printf(1, "====================================================================\n");
  printf(1, "Best Performer Analysis:\n");
  printf(1, "  * Optimal achieves the lowest page faults (9 on 3 frames, 8 on 4\n");
  printf(1, "    frames, 7 on 5 frames) and highest hit ratio (55.0%% -> 65.0%%)\n");
  printf(1, "    by evicting the page whose next reference is farthest in future.\n");
  printf(1, "  * Among practical online algorithms, LRU outperforms FIFO across\n");
  printf(1, "    all frame counts (12 vs 15 faults on 3 frames) and matches\n");
  printf(1, "    Optimal on 4 and 5 frames.\n");
  printf(1, "====================================================================\n");
}

int
main(int argc, char *argv[])
{
  int refs[MAX_REFS];
  int nrefs = 20;
  int i, f_fifo, f_lru, f_opt;

  for(i = 0; i < nrefs; i++)
    refs[i] = default_refs[i];

  printf(1, "====================================================================\n");
  printf(1, "Question 4: Page Replacement — FIFO, LRU, and Optimal (pagereplace)\n");
  printf(1, "====================================================================\n");
  printf(1, "Reference String (%d pages): ", nrefs);
  for(i = 0; i < nrefs; i++)
    printf(1, "%d ", refs[i]);
  printf(1, "\n");

  // Task (a) & (b): Run all three algorithms with 3 frames
  f_fifo = run_fifo(refs, nrefs, 3, 1);
  f_lru  = run_lru(refs, nrefs, 3, 1);
  f_opt  = run_optimal(refs, nrefs, 3, 1);

  printf(1, "\nTask (b) Summary — Total Page Faults with 3 Frames:\n");
  printf(1, "  FIFO    : %d page faults (%d hits)\n", f_fifo, nrefs - f_fifo);
  printf(1, "  LRU     : %d page faults (%d hits)\n", f_lru,  nrefs - f_lru);
  printf(1, "  Optimal : %d page faults (%d hits)\n", f_opt,  nrefs - f_opt);

  // Task (c): Run with 3, 4, and 5 frames and display comparison table
  print_comparison_table(refs, nrefs);

  exit();
}
