#ifndef _PAGE_FAULT_HANDLER_H_
#define _PAGE_FAULT_HANDLER_H_
#include "page_table.h"


/* init_vmm
 *   called exactly once when the simulation starts
 *   so you can do any initialization you need to do
 */
void init_vmm(void);


/* cleanup_vmm
 *   called exactly once just before the simulation finished
 *   so you can do any cleanup you need to do
 */
void cleanup_vmm(void);


/* context_switch
 *   called when a context switch happens, so you can update state as needed
 *
 * new_proc - PID of the new process that is about to start running on the CPU
 *
 * (N.B. postcondition omitted so you can figure out what state needs to be updated)
 */
void context_switch(pid_t pid);


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
void page_fault_handler(pid_t faulting_proc, virt_addr_t faulting_addr);

#endif // _PAGE_FAULT_HANDLER_H_
