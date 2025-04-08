STARTER_DIR=/comp/111/assignments/vmm
CC=gcc
LD=$(CC)
CPPFLAGS=-g -std=gnu11 -Wpedantic -Wall -Wextra #-DDEBUG
CFLAGS=-I.
LDFLAGS=
LDLIBS=-lm
OBJECTS=physical_memory.o swap_file.o mmu.o page_fault_handler.o vmm.o
PROGRAMS=vmm

all: $(PROGRAMS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

vmm: $(OBJECTS)
	$(LD) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY:
clean:
	rm -f *.o $(PROGRAMS)
