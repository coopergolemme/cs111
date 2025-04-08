#include "physical_memory.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

byte_t* physical_memory = NULL;

void init_physical_memory(uint8_t addr_len) {
  uint16_t mem_size = pow(2, addr_len) * sizeof(byte_t);
  physical_memory = malloc(mem_size);
  assert (NULL != physical_memory); // check for out of memory
  memset(physical_memory, 0, mem_size);
}

void store(phys_addr_t addr, byte_t data) {
  physical_memory[addr] = data;
}

byte_t load(phys_addr_t addr) {
  return physical_memory[addr];
}

byte_t* get_frame_base_addr(pfn_t pfn) {
  return &physical_memory[pfn << (unsigned int) log2(PAGE_SIZE)];
}

void zero_frame(pfn_t pfn) {
  memset(get_frame_base_addr(pfn), 0, PAGE_SIZE * sizeof(byte_t));
}

void cleanup_physical_memory(void) {
  free(physical_memory);
  physical_memory = NULL;
}
