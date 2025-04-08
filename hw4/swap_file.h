#ifndef _SWAP_FILE_H_
#define _SWAP_FILE_H_
#include "common.h"

void init_swap_file(void);
void cleanup_swap_file(void);
bool_t save_page_to_disk(pfn_t data, pid_t pid, vpn_t vpn);
bool_t load_page_from_disk(pid_t pid, vpn_t vpn, pfn_t data);

#endif // _SWAP_FILE_H_
