#include "physical_memory.h"
#include "swap_file.h"
#include "page_fault_handler.h"
#include "mmu.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define FILE_LINE_SIZE 1024 // size of the line buffer for read a line of a file
#define WHITESPACE_DELIM " \t\r\n" // whitespace characters to use as a delimiter

uint16_t PAGE_SIZE = 16; // size of a memory page, in bytes
uint8_t  ADDR_LEN = 12;  // number of bits in a memory address
                         // (virtual and physical addrs are the same length)


int main(int argc, char** argv) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: ./vmm workload_file\n");
    return EXIT_FAILURE;
  }

  pid_t current_proc = -1;
  unsigned long num_page_faults = 0;

  init_physical_memory(ADDR_LEN);
  init_swap_file();
  init_vmm();

  char line[FILE_LINE_SIZE];
  FILE* file = fopen(argv[1], "r");
  if (NULL == file) {
    perror("ERROR opening file");
    fprintf(stderr, "Filename: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  
  // Run each line in the workload file
  while (NULL != fgets(line, FILE_LINE_SIZE, file)) {

    // Get the memory operation
    char* operation = strtok(line, WHITESPACE_DELIM);
    if (NULL == operation) {
      perror("ERROR in file contents");
      fprintf(stderr, "Memory operation not found on workload file line: %s\n", line);
      fclose(file);
      return EXIT_FAILURE;
    }

    // Get the process ID
    char* token = strtok(NULL, WHITESPACE_DELIM);
    if (NULL == token) {
      perror("ERROR in file contents");
      fprintf(stderr, "Process ID not found on workload file line: %s\n", line);
      fclose(file);
      return EXIT_FAILURE;
    }

    char* endptr = NULL;
    pid_t pid = strtoul(token, &endptr, 10);
    if ('\0' != *endptr) {
      perror("ERROR in file contents");
      fprintf(stderr, "Failed to convert string \"%s\" to process ID (decimal integer)\n", token);
      fclose(file);
      return EXIT_FAILURE;
    }

    // Context Switch
    if (pid != current_proc) {
      context_switch(pid);
      current_proc = pid;
    }

    // Get the virtual address
    token = strtok(NULL, WHITESPACE_DELIM);
    if (NULL == token) {
      perror("ERROR in file contents");
      fprintf(stderr, "Memory address not found on workload file line: %s\n", line);
      fclose(file);
      return EXIT_FAILURE;
    }

    endptr = NULL;
    virt_addr_t virt_addr = strtoul(token, &endptr, 16);
    if ('\0' != *endptr) {
      perror("ERROR in file contents");
      fprintf(stderr, "Failed to convert string \"%s\" to memory address (hexadecimal integer)\n", token);
      fclose(file);
      return EXIT_FAILURE;
    }

    // STORE operation
    if ('S' == operation[0] || 's' == operation[0]) {
      // Get the data
      token = strtok(NULL, WHITESPACE_DELIM);
      if (NULL == token) {
        perror("ERROR in file contents");
        fprintf(stderr, "Data not found on workload file line: %s\n", line);
        fclose(file);
        return EXIT_FAILURE;
      }

      endptr = NULL;
      byte_t data = strtoul(token, &endptr, 0);
      if ('\0' != *endptr) {
        perror("ERROR in file contents");
        fprintf(stderr, "Failed to convert string \"%s\" to data (uint8)\n", token);
        fclose(file);
        return EXIT_FAILURE;
      }

      // Translate the address
      phys_addr_t phys_addr = translate_addr(virt_addr, TRUE);
      while (PAGE_FAULT == phys_addr) {
        printf("PAGE FAULT while accessing PID: %d, Virt Addr: 0x%.4x\n",
               pid, virt_addr);
        num_page_faults++;
        page_fault_handler(pid, virt_addr);
        phys_addr = translate_addr(virt_addr, TRUE);
      }

      // Now store it
      assert(PAGE_FAULT != phys_addr);
      store(phys_addr, data);
      printf("STORE PID: %d, Virt Addr: 0x%.4x, Phys Addr: 0x%.4x, Data: %d\n",
             pid, virt_addr, phys_addr, data);

    // LOAD operation
    } else if ('L' == operation[0] || 'l' == operation[0]) { // LOAD
      // Translate the address
      phys_addr_t phys_addr = translate_addr(virt_addr, FALSE);
      while (PAGE_FAULT == phys_addr) {
        printf("PAGE FAULT while accessing PID: %d, Virt Addr: 0x%.4x\n",
               pid, virt_addr);
        num_page_faults++;
        page_fault_handler(pid, virt_addr);
        phys_addr = translate_addr(virt_addr, FALSE);
      }

      // Now load it
      byte_t data = load(phys_addr);
      printf("LOAD PID: %d, Virt Addr: 0x%.4x, Phys Addr: 0x%.4x, Data: %d\n",
             pid, virt_addr, phys_addr, data);

    // Error (neither STORE nor LOAD)
    } else {
      fprintf(stderr, "Invalid operation \"%s\" on workload file line: %s\n", token, line);
      fclose(file);
      return EXIT_FAILURE;
    }
  }

  // Do not close file here, because we need ferror and feof on the last fgets
  if (ferror(file)) {
    perror("ERROR reading file");
    fclose(file);
    return EXIT_FAILURE;
  } else if (!feof(file)) {
    perror("ERROR did not reach end of file");
    fclose(file);
    return EXIT_FAILURE;
  }

  printf("Total Number of Page Faults: %lu\n", num_page_faults);

  fclose(file);
  file = NULL;
  cleanup_vmm();
  cleanup_swap_file();
  cleanup_physical_memory();
  return 0;
}
