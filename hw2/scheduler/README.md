# Programming Assignment 2: Scheduler

In this assignment, you’ll be implementing several scheduling policies on top of an event-based process simulator that we are providing you. The starter code takes an input file with descriptions of processes and simulates their running, including events such as process arrival (i.e., fork()), blocking, unblocking, time slices ending (for relevant scheduling policies), and process termination. Each of these represents a trap or interrupt that calls the scheduler code in the kernal. You will write that scheduler code (although it will be simulated in user mode, rather than actually running in the kernel).

# #Overview

scheduler.h contains the function declarations for all the functions you are required to implement, as well as several provided functions that you may call to help you out. You will write your actual implementations in each of the sched*\*.c files – one scheduling algorithm in each file. In addition to the required functions, you are encouraged to write some helper functions in your sched*\*.c files. Here is a brief overview of each function you are required to implement, but see the function comments in scheduler.h for more thorough descriptions of the functions.

function description
sched_init initialization code can go here; this will be called exactly once before any other functions
sched_cleanup clean up code can go here; this will be called exactly once after all other functions have been called
sched_new_process called whenever a new process arrives (i.e., is created by simulated-fork())
sched_finished_time_slice called whenever the currently running process’ time slice is over
sched_blocked called whenver the currently running process blocks (i.e., begins an I/O operation
sched_unblocked called whenver a process (not currently running) unblocks and becomes eligible to run again
sched_terminated called whenever a process terminates

In order to implement your schedulers, the simulator provides the following functions that you can call:

function description
int context_switch(pid_t pid) changes to process running on the CPU to the one identified by the given process ID
pid_t get_current_proc() returns the process ID of the process currently running on the CPU
time_ticks_t get_time_slice() returns the duration of one time slice (specified by the input file)
void use_time_slice(bool_t use) tells the simulator whether it should use time slices (TRUE) or not (FALSE)
`void print_process(const struct process\* proc) prints a process struct to stderr; potentially helpful for debugging
Some info on provided data types:

bool_t is a boolean type that can have values TRUE or FALSE (all caps)
pid_t is the type of a process ID, as a signed integer
time_ticks_t is an amount of time, as an unsigned integer, in the simulator’s internal time units (These time units are abstract, since this is just a simulation, so they don’t correspond to real time. You may think of them as milliseconds, or microseconds, or whatever you want, but we’ll just call them “ticks”.)
You can also find the following relevant data structures defined in process.h

### Representing a process:

struct process {
pid_t pid;
state_t state;
unsigned int tickets;
time_ticks_t arrival_time;
struct burst\* current_burst;
};

The possible state values are: NOT_ARRIVED, READY, BLOCKED, TERMINATED

arrival_time is the time from the beginning of the simulation (in ticks) when the process arrives. At that time, it will change state from NOT_ARRIVED to READY and sched_new_process() will be called.

tickets is the number of tickets the process has for scheduling policies that use tickets, but may safely be ignored when implementing policies that don’t use tickets.

Each process has a series of “bursts”, which is an amount of contiguous work it wants to do on the CPU (a “CPU burst”) or an amount of time it will spend doing I/O (an “I/O burst”) and be BLOCKED while doing so. current_burst is the head of a linked list of the reamining bursts a process wants to do before it terminates. They will alternate bewteen CPU_BURST and IO_BURST, calling sched_blocked() and sched_unblocked() when switching between them. The first burst will always be a CPU_BURST (although current_burst indicates the current, not the first burst, so it is only guaranteed to be a CPU_BURST when the process arrives), and the last burst will always be a CPU_BURST.

struct burst {
burst_type_t type;
time_ticks_t remaining_time;
struct burst\* next_burst;
};

type is the burst type, either CPU_BURST or IO_BURST

remaining_time is the amount of time (in ticks) remaining in the current burst (the simulator will automaticall decrease this value when bursts are only partially completed).

next_burst is a pointer to the next burst struct in the linked list, or NULL at the end of the list.

## Implementation

sched_rr.c – a Round Robin scheduler
sched_stcf.c – a Shortest Time to Completion First (STCF) scheduler
sched_stride.c – a Stride Scheduler with a constant C=1,000,000

A Makefile is provided that will build a separate executable for each scheduling algorith (e.g., sched_rr built from sched_rr.c and linked with the simulator code). The program will take one command line argument, which will be the name of a file with information about the processes to simulate, so you can run it like this:

./sched_rr test.proc

Where test.proc is a text file that looks something like this:

20
6
1000 0 10 20 30
1000 0 30 20 10
1000 10 20 30 10
1500 10 20 10 30
2000 50 10 30 20
2000 100 30 10 20
The file format (which you want to understand so you can create your own tests) works like this:

The first line is the time slice value to use
This should be ignored by algorithms that don’t use time slices, but is still included in the input file nonetheless for consistency, and so the same input file can potentially work with multiple different schedulers.
20 in the example file above
The second line is how many processes will be simulated (which is also the number of lines after it in the file)
6 in the example file above
Following the second line are that many lines in the file that describe the processes, one process per line
1000 0 10 20 30, for example
The format of each process line is <tickets> <arrival time> <CPU burst> <IO burst> <CPU burst> ...

<tickets> is the number of tickets assigned to this process
This should be ignored by algorithms that don’t use tickets, but is still included in the input file nonetheless for consistency, and so the same input file can potentially work with multiple different schedulers.
2000 on the last line of the example file above
<arrival time> is the time (in ticks) when this process will arrive (before which time, the process does not actually exist, for all practical purposes, even though the simulator is still tracking it)
The simultor starts at t=0 ticks, so this is also how much time passes from the beginning of the simulation until this process arrives.
100 on the last line of the example file above
After those two, there will be an odd number of additional numbers, representing the burst time (in ticks) of each of the process’ bursts.
The first bust will always be a CPU burst (30 on the last line of the example file above
Bursts alternate between CPU and I/O bursts (so 10 on the last line of the example file above is an I/O burst, and 20 is a CPU burst)
The last burst must always be a CPU burst (hence a valid input file must have an odd number of bursts for each process)
After a process finishes its last burst, it terminates (and this is the only way for a process to terminate in this simulation, although things like “kill” signals do exist in the real world)
Processes do not need to have the same number of bursts as each other (despide the example above showing then with 3 bursts each)
A process may have as few as 1 burst, in which case it never blocks for I/O, and just runs its one CPU burst and then terminates
This should help you produce some workload files for yourself that you can use to test your code. We may provide some additional ones for you to use, but don’t count on the provided ones to give sufficiently thorough testing on their own.

Additional Implementation Details
You must not print any output to stdout. If you want to print some debug output, be sure to print it to stderr instead. All output to stdout should come from the provided simulator code, not from the code that you write. When we grade it, we will use input files that should give an exact correct output on stdout if each scheduling algorithm is implemented correctly. We will be looking for an exact match for the expected output on stdout, which additional output on stdout that’s not from the simulator code would corrupt (but we will ignore output on stderr, so stderr is safe to use, if you want).
You may not use any of the “forbidden functions”, which are:
strcpy, strcat, gets, sprintf, vsprintf
Some alternatives you may use include: strncpy, strncat, fgets, snprintf, vsnprintf
Also, follow the CERN Common vulnerabilities guide for C programmers advice
Test your code with valgrind as well. Memory leaks will be considered bugs. Other memory errors reported by valgrind (i.e., that are not leaks) will not impact your grade, but they could provide helpful information in tracking down incorrect behavior bugs (which will impact your grade by causing test cases to fail).
Submitting Your Program
On one of the class VMs, run the following command to submit:

provide comp111 scheduler sched_rr.c sched_stcf.c sched_stride.c
The submission program will attempt to compile your program using a Makefile we have created that ensures your code is linked to the original, unmodified simulation starter code. You are not submitting a Makefile, because we will be providing the Makefile for your submitted code, in order to ensure that it compiles in this way. However, if you code compiles correctly with the Makefile we provided you together with the starter code, then it should compile correctly using the Makefile we use on your submitted code. Your program must properly compile or the submission program will reject your program. Programs that fail to compile will not be graded.

See the submission instructions on the first assignment for more details about what happens when you run the submission command.

To resubmit, you can just run the submission script again. It does not overwrite your previous submissions; we will have copies of all of them, but only your last submission counts for grading.

Grading
We will test your code for correctness by running it with a set of different input files that we’ve created. The output on stdout will be compared for an exact match to the expected output for each input file on each scheduler. Any output on stderr will be ignored, however. We will only use your sched\_\*.c files, together with our own copies of all the other files, so if you modify any of the other files, the modified version won’t get used when we grade.

Grading will be based on:

passing the test cases (i.e., output matches for all the testing input files)
compiling successfully is a precondition to this (and also to being able to submit)
no crashes (segfaults, etc.)
no memory leaks reported by valgrind on any of the test cases
Other memory errors do not affect your grade, only memory leaks. So if valgrind outputs “All heap blocks were freed – no leaks are possible”, then you’re okay.
no output to standard output from you code
does not use any “forbidden functions” (see above)
follows all the other assignment parameters specified above
code inspection (relatively light compared to previous assignments, though)
as before, just be sure your code style is not a hot mess 😃
