#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_
#include "common.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>


/* Page Table Entry
 *   a single entry in a page table, representing one memory page
 */
typedef struct {
  pfn_t pfn;        // physical frame number
  bool_t present;   // is page present in memory? (AKA "valid" bit)
  bool_t reference; // has page been accessed?
  bool_t dirty;     // has page been written to?
} pte_t;


/* zero_frame
 *   clears the memory (i.e., "zeros" it) of the specified physical memory frame
 *
 * pfn - physical frame number (PFN) of the frame to clear
 */
void zero_frame(pfn_t pfn);

#endif // _PAGE_TABLE_H_
