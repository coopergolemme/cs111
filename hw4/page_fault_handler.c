#include "page_fault_handler.h"
#include "mmu.h"
#include "swap_file.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>


/* Process Control Block (PCB)
 *   stores data relevant to a single process
 *   (all we really care about for this simulation is the page table)
 */
typedef struct {
  pte_t* page_table; // base address of this proc's page table
} pcb_t;

pcb_t pcb[PID_MAX]; // we need a PCB for each proc


/* init_vmm
 *   called exactly once when the simulation starts
 *   so you can do any initialization you need to do
 */
void init_vmm(void) {
  // This implementation provides a static pre-filled page table,
  // with the first 8 pages (VPN 0 - 7) present and mapped to PFNs.
  // This allows testing address translations alone (mmu.c),
  // without having to implment anything in this file as well,
  // but ultimately will need to be replaced by proper page fault handling.

  ptbr = malloc(NUM_PAGES * sizeof(pte_t));
  assert(NULL != ptbr); // chech for Out of Memory error
  memset(ptbr, 0, NUM_PAGES * sizeof(pte_t));

  pfn_t pfn[8] = { 0x00, 0x01, 0x03, 0x02, 0x04, 0x07, 0x06, 0x05 };
  for (vpn_t vpn = 0; vpn < 8; ++vpn) {
    ptbr[vpn].pfn = pfn[vpn];
    ptbr[vpn].present = TRUE;
  }

  // TODO: replace the above code with initializations that you need to do
}


/* cleanup_vmm
 *   called exactly once just before the simulation finished
 *   so you can do any cleanup you need to do
 */
void cleanup_vmm(void) {
  // Cleanup corresponding to the provied init implementation above,
  // but this will also need to be replaced when the init is changed
  // to its final verion.
  free(ptbr);
  ptbr = NULL;

  // TODO: replace the above code with cleanup that you need to do
}


/* context_switch
 *   called when a context switch happens, so you can update state as needed
 *
 * new_proc - PID of the new process that is about to start running on the CPU
 *
 * (N.B. postcondition omitted so you can figure out what state needs to be updated)
 */
void context_switch(pid_t new_proc) {
  // TODO
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
pfn_t find_free_frame(void) {
  // TODO
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
pfn_t find_victim_page(void) {
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
void page_fault_handler(pid_t faulting_proc, virt_addr_t faulting_addr) {
  // TODO
}
