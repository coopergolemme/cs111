#include "mmu.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

pte_t *ptbr = NULL; // Page Table Base Register
                    // points to the start of the current page table

/* get_vpn
 *   (optional helper function)
 *   gets the VPN from a virtual address
 *
 * addr - the virtual address
 *
 * returns the VPN from the addr
 */
vpn_t get_vpn(virt_addr_t addr)
{
  // Number of bits for the offset depends on the page size
  int offset_bits = log2(PAGE_SIZE);
  // Shift over to get higher order bits which correspond to vpn
  vpn_t vpn = addr >> offset_bits;
  return vpn;
}

/* get_offset
 *   (optional helper function)
 *   gets the page offset from a virtual address
 *
 * addr - the virtual address
 *
 * returns the page offset from the addr
 */
offset_t get_offset(virt_addr_t addr)
{
  int offset_bits = log2(PAGE_SIZE);
  // make mask of all 1's offset in width
  int mask = (1 << offset_bits) - 1;
  // and to get the lower order bits for offset
  offset_t offset = addr & mask;
  return offset;
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
phys_addr_t translate_addr(virt_addr_t virt_addr, bool_t write)
{

  vpn_t vpn = get_vpn(virt_addr);
  offset_t offset = get_offset(virt_addr);

  pte_t *pte = &ptbr[vpn];

  // If the page is not present in memory, return PAGE_FAULT
  if (!pte->present)
  {
    return PAGE_FAULT;
  }

  // Mark the page as accessed 
  pte->reference = TRUE;

  // If this is a write operation, mark the page as dirty
  if (write)
  {
    pte->dirty = TRUE;
  }

  // Compute the physical address
  int offset_bits = log2(PAGE_SIZE);
  phys_addr_t phys_addr = (pte->pfn << offset_bits) | offset;

  return phys_addr;
}
