STARTER_DIR=/comp/111/assignments/vmm
CC=gcc
LD=$(CC)
CPPFLAGS=-g -std=gnu11 -Wpedantic -Wall -Wextra #-DDEBUG
CFLAGS=-I$(STARTER_DIR)
LDFLAGS=
LDLIBS=-lm
OBJECTS=physical_memory.o swap_file.o mmu.o page_fault_handler.o vmm.o
PROGRAMS=vmm

all: $(PROGRAMS)

physical_memory.o: $(STARTER_DIR)/physical_memory.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

swap_file.o: $(STARTER_DIR)/swap_file.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

vmm.o: $(STARTER_DIR)/vmm.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

vmm: $(OBJECTS)
	$(LD) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY:
clean:
	rm -f *.o $(PROGRAMS)
