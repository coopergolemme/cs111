#include "page_fault_handler.h"
#include "mmu.h"
#include "swap_file.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_FRAMES NUM_PAGES

typedef struct
{
  bool_t is_used; // TRUE if the frame is in use, FALSE otherwise
  vpn_t vpn;      // Virtual Page Number mapped to this frame
  pid_t owner;    // Process ID of the owner of this frame
} frame_t;

frame_t *frame_table;

/* Process Control Block (PCB)
 *   stores data relevant to a single process
 *   (all we really care about for this simulation is the page table)
 */
typedef struct
{
  pte_t *page_table; // base address of this proc's page table
} pcb_t;

pcb_t pcb[PID_MAX]; // we need a PCB for each proc

void init_frame_table();
void init_page_table();

/* init_vmm
 *   called exactly once when the simulation starts
 *   so you can do any initialization you need to do
 */
void init_vmm(void)
{
  // This implementation provides a static pre-filled page table,
  // with the first 8 pages (VPN 0 - 7) present and mapped to PFNs.
  // This allows testing address translations alone (mmu.c),
  // without having to implment anything in this file as well,
  // but ultimately will need to be replaced by proper page fault handling.

  ptbr = malloc(NUM_PAGES * sizeof(pte_t));
  assert(NULL != ptbr); // chech for Out of Memory error
  init_page_table();

  frame_table = malloc(NUM_FRAMES * sizeof(frame_t));
  assert(NULL != frame_table);
  init_frame_table();
}

void init_frame_table()
{
  // Initialize each frame in the frame table
  for (pfn_t pfn = 0; pfn < NUM_FRAMES; pfn++)
  {
    frame_table[pfn].is_used = FALSE; // Frame is not in use
    frame_table[pfn].vpn = 0;         // Default VPN (placeholder)
    frame_table[pfn].owner = -1;      // No owner (invalid PID)
  }

  printf("Frame table initialized with %d frames.\n", NUM_FRAMES);
}

void init_page_table()
{
  for (pid_t pid = 0; pid < PID_MAX; pid++)
  {
    pcb[pid].page_table = malloc(NUM_PAGES * sizeof(pte_t));
    assert(NULL != pcb[pid].page_table); // Check for out-of-memory errors

    // Initialize each page table entry
    for (vpn_t vpn = 0; vpn < NUM_PAGES; vpn++)
    {
      pcb[pid].page_table[vpn].pfn = 0;
      pcb[pid].page_table[vpn].present = FALSE;
      pcb[pid].page_table[vpn].reference = FALSE;
      pcb[pid].page_table[vpn].dirty = 0;
    }
  }

  printf("Page tables initialized for %d processes.\n", PID_MAX);
}
/* cleanup_vmm
 *   called exactly once just before the simulation finished
 *   so you can do any cleanup you need to do
 */
void cleanup_vmm(void)
{
  // Cleanup corresponding to the provied init implementation above,
  // but this will also need to be replaced when the init is changed
  // to its final verion.
  free(ptbr);
  ptbr = NULL;

  free(frame_table);
  frame_table = NULL;

  // TODO: replace the above code with cleanup that you need to do
}

/* context_switch
 *   called when a context switch happens, so you can update state as needed
 *
 * new_proc - PID of the new process that is about to start running on the CPU
 *
 * (N.B. postcondition omitted so you can figure out what state needs to be updated)
 */
void context_switch(pid_t new_proc)
{
  ptbr = pcb[new_proc].page_table;
}

/* find_free_frame
 *   helper function to search for a free frame of physical memory
 *   if multiple frames are free, always choose the one with the lowest PFN
 *
 * returns the PFN of the free frame with the lowest PFN
 *   or -1 if no frames are free
 *
 * postcondition: the returned frame does NOT contain a page
 *   (i.e., is_used is FALSE)
 */
pfn_t find_free_frame(void)
{
  // Iterate over all physical frames
  for (pfn_t pfn = 0; pfn < NUM_FRAMES; pfn++)
  {
    // Check if the frame is free (not in use)
    if (!frame_table[pfn].is_used)
    {
      return pfn; // Return the PFN of the free frame
    }
  }

  // If no free frame is found, return -1
  return -1;
}

/* find_victim_page
 *   implements the page replacement policy by choosing a victim page
 *   (does not actually evict the victim, just chooses it; eviction is the caller's responsibility)
 *
 * returns the PFN of the frame containing the chosen victim page
 *
 * precondition: there are currently no free frames in physical memory
 *
 * postcondition: the returned frame contains a page (i.e., is_used is TRUE)
 */
pfn_t find_victim_page(void)
{
  // TODO
  // Clock Sweep
  return 0;
}

/* page_fault_handler
 *   this is called when a page fault occurs
 *   in other words, when an address translation was attempted on a page
 *   whose present (valid) bit is FALSE, indicating that it is not stored
 *   in physical memory
 *
 * faulting_proc - PID of the process whose page was trying to be accessed
 * faulting_addr - virtual address that cause the fault
 *
 * precondition: the memory page containing faulting_addr is NOT stored in memory
 *   and the present (valid) bit is FALSE in faulting_proc's page table
 *
 * postcondition: the memory page containing faulting_addr is stored in memory
 *   and the present (valid) bit is TRUE in faulting_proc's page table
 *
 * (N.B. the page_fault_handler may need to update other state as well,
 *   but postconditions for those have not been documented so you can
 *   figure out what you need to do on your own.)
 */
void page_fault_handler(pid_t faulting_proc, virt_addr_t faulting_addr)
{
  // printf("Page fault handler\n");
  pfn_t free_frame = find_free_frame();
  // if (free_frame == -1)
  // {
  //   printf("No free frame found. Need to evict a page.\n");
  //   free_frame = find_victim_page();
  // }
  // else
  // {
  //   // printf("Free frame found: PFN %d\n", free_frame);
  // }

  // need to handle no free frame found

  // load faulting page into free frame
  vpn_t faulting_vpn = get_vpn(faulting_addr);
  load_page_from_disk(faulting_proc, faulting_vpn, free_frame);

  // Update the page table of the faulting process
  pcb[faulting_proc].page_table[faulting_vpn].pfn = free_frame;
  pcb[faulting_proc].page_table[faulting_vpn].present = TRUE;
  pcb[faulting_proc].page_table[faulting_vpn].dirty = FALSE; // Reset dirty bit

  // Update the frame tableb
  frame_table[free_frame].is_used = TRUE;
  frame_table[free_frame].vpn = faulting_vpn;
  frame_table[free_frame].owner = faulting_proc;
}
