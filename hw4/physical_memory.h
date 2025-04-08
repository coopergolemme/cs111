#ifndef _PHYSICAL_MEMORY_H_
#define _PHYSICAL_MEMORY_H_
#include "common.h"
#include <stdint.h>

typedef uint16_t phys_addr_t; // physical memory address

void init_physical_memory(uint8_t addr_len);
void cleanup_physical_memory(void);


/* store
 *   stores the provided data at the specificed memory address
 *
 * addr - address where the data should be stored
 * data - data to store in the address
 */
void store(phys_addr_t addr, byte_t data);


/* load
 *   retrieves data from the specificed address
 *
 * addr - address from which the data should be loaded
 *
 * returns the data
 */
byte_t load(phys_addr_t addr);


/* get_frame_base_addr
 *   gets a pointer to the internal simulation memory for a simulated physical frame
 *
 * pfn - physical frame number (PFN) of the requested frame
 *
 * return a pointer to the internal simulation memory where the simulated frame is stored
 */
byte_t* get_frame_base_addr(pfn_t pfn);

#endif // _PHYSICAL_MEMORY_H_
