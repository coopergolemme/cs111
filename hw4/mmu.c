#include "mmu.h"
#include <math.h>
#include <assert.h>


pte_t* ptbr = NULL; // Page Table Base Register
                    // points to the start of the current page table


/* get_vpn
 *   (optional helper function)
 *   gets the VPN from a virtual address
 *
 * addr - the virtual address
 *
 * returns the VPN from the addr
 */
vpn_t get_vpn(virt_addr_t addr) {
  // TODO
  return 0;
}


/* get_offset
 *   (optional helper function)
 *   gets the page offset from a virtual address
 *
 * addr - the virtual address
 *
 * returns the page offset from the addr
 */
offset_t get_offset(virt_addr_t addr) {
  // TODO
  return 0;
}


/* translate_addr
 *   translates a virtual memory address to a physical memory address
 *   called when any memory operation (load or store) is executed
 *
 * virt_addr - the virtual memory address to translate
 * write - is the memory access a store operation?
 *
 * returns the phsyical memory address where the virtual addr is stored
 *   or PAGE_FAULT if the virtual page is not currently in memory
 */
phys_addr_t translate_addr(virt_addr_t virt_addr, bool_t write) {
  // TODO
  return 0;
}
