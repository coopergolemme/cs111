#ifndef _COMMON_H_
#define _COMMON_H_
#include <math.h>
#include <unistd.h>
#include <stdint.h>


typedef char bool_t; // boolean data type
#define TRUE 1
#define FALSE 0

typedef uint8_t byte_t;      // one byte of data, data type
                             // (this system is byte addressable)

typedef uint16_t    virt_addr_t; // virtual memory address data type
typedef virt_addr_t vpn_t;       // virtual page number data type
typedef virt_addr_t pfn_t;       // physical frame number data type


extern uint16_t PAGE_SIZE; // size of a memory page, in bytes
extern uint8_t  ADDR_LEN;  // number of bits in a memory address
                           // (virtual and physical addrs are the same length)

// number of virtual pages in an address space
#define NUM_PAGES ((uint16_t)pow(2, ADDR_LEN - log2(PAGE_SIZE)))

#define PID_MAX 128 // maximum number of processes there can be
                    // assume PIDs will not be reused
                    // (e.g., pid 7 will not terminate and then be assigned to
                    // a new process that arrives later, making it also pid 7)

#endif // _COMMON_H_
