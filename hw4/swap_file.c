#include "swap_file.h"
#include "physical_memory.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

byte_t*** pages = NULL;

void init_swap_file(void) {
  pages = malloc(PID_MAX * sizeof(byte_t**));
  assert(NULL != pages); // check for out of memory
  for (pid_t pid = 0; pid < PID_MAX; ++pid) {
    pages[pid] = malloc(NUM_PAGES * sizeof(byte_t*));
    assert(NULL != pages[pid]); // check for out of memory
    memset(pages[pid], 0, NUM_PAGES * sizeof(byte_t*));
  }
}

void cleanup_swap_file(void) {
  for (pid_t pid = 0; pid < PID_MAX; ++pid) {
    assert(NULL != pages[pid]);

    for (vpn_t vpn = 0; vpn < NUM_PAGES; ++vpn)
      if (NULL != pages[pid][vpn]) {
        free(pages[pid][vpn]);
        pages[pid][vpn] = NULL;
      }

    free(pages[pid]);
    pages[pid] = NULL;
  }

  assert(NULL != pages);
  free(pages);
  pages = NULL;
}

bool_t save_page_to_disk(pfn_t data, pid_t pid, vpn_t vpn) {
  // PRECONDITION: the page's data is stored in the specified physical memory frame
  if (NULL == pages[pid][vpn]) {
    pages[pid][vpn] = malloc(PAGE_SIZE * sizeof(byte_t));
    memset(pages[pid][vpn], 0, PAGE_SIZE * sizeof(byte_t));
  }
  // INVARIANT: swap space for this page is allocated (pages[pid][vpn] is not NULL)
  
  memcpy(pages[pid][vpn], get_frame_base_addr(data), PAGE_SIZE * sizeof(byte_t));
  // INVARIANT: the page is stored on disk
  printf("DISK saved page PID: %d, VPN: 0x%.3x to swap file\n", pid, vpn);
  return TRUE;
}

bool_t load_page_from_disk(pid_t pid, vpn_t vpn, pfn_t data) {
  if (NULL == pages[pid][vpn])
    return FALSE;
  // INVARIANT: the page is stored on disk

  memcpy(get_frame_base_addr(data), pages[pid][vpn], PAGE_SIZE * sizeof(byte_t));
  // INVARIANT: the page's data is stored in the specified physical memory frame
  printf("DISK loaded page PID: %d, VPN: 0x%.3x from swap file\n", pid, vpn);
  return TRUE;
}
