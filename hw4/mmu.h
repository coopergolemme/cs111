#ifndef _MMU_H_
#define _MMU_H_
#include "page_table.h"
#include "physical_memory.h"

#define PAGE_FAULT ((phys_addr_t)-1)

typedef virt_addr_t offset_t;

extern pte_t* ptbr; // Page Table Base Register
                    // points to the start of the current page table

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
phys_addr_t translate_addr(virt_addr_t virt_addr, bool_t write);


/* get_vpn
 *   helper function that gets the VPN from a virtual address
 *
 * addr - the virtual address
 *
 * returns the VPN from the addr
 */
vpn_t get_vpn(virt_addr_t addr);


/* get_offset
 *   (optional helper function)
 *   gets the page offset from a virtual address
 *
 * addr - the virtual address
 *
 * returns the page offset from the addr
 */
offset_t get_offset(virt_addr_t addr);

#endif // _MMU_H_
