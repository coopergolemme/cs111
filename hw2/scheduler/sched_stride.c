#include "scheduler.h"
#include "process.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/********************
 * STRIDE Scheduler *
 ********************/

#define STRIDE_CONSTANT 1000000

void run_new_process();

/**
 * proc_list
 *    A linked list of processes, with stride and pass values for each process
 */
struct proc_list
{
  int stride;
  int pass;
  struct process *proc;
  struct proc_list *next;
};

static struct proc_list *head = NULL;

/**
 * proc_list_empty
 * Description: checks if the process list is empty or if all of the processes are terminated or blocked.
 * Returns: bool - true if empty or all process are terminated or blocked, false otherwise
 */
bool proc_list_empty()
{
  if (head == NULL)
  {
    return true;
  }
  struct proc_list *current = head;

  while (current != NULL)
  {
    if (current->proc->state != TERMINATED && current->proc->state != BLOCKED)
    {
      return false;
    }
    current = current->next;
  }

  return true;
}

/**
 * is_only_one_ready
 * Description: checks if there is only one process in the list that is in the READY state
 * Returns: bool - true if there is exactly one READY process, false otherwise
 */
bool is_only_one_ready()
{
  struct proc_list *current = head;
  int ready_count = 0;

  while (current != NULL)
  {
    if (current->proc->state == READY)
    {
      ready_count++;
      if (ready_count > 1)
      {
        return false;
      }
    }
    current = current->next;
  }

  return ready_count == 1;
}

/**
 *  add_process
 *  Description: adds a process to the process list
 *  Parameters:
 *    proc - the process to add
 *    stride - the stride value for the process
 *    pass - the pass value for the process
 * Returns: void
 */
void add_process(struct process *proc, int stride, int pass)
{
  struct proc_list *new_node = (struct proc_list *)malloc(sizeof(struct proc_list));
  new_node->proc = proc;
  new_node->stride = stride;
  new_node->pass = pass;
  new_node->next = NULL;
  if (head == NULL)
  {
    head = new_node;
  }
  else
  {
    struct proc_list *current = head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    current->next = new_node;
  }
}

/**
 * increment_pass
 * Description: increments the pass value of the process with the given pid
 * Parameters: pid_t pid - the process id of the process to increment the pass
 *                        value of the process to increment the pass value of
 * Returns: void
 */
void increment_pass(pid_t pid)
{
  // printf("PID: %d\n", pid);
  struct proc_list *current = head;
  while (current != NULL)
  {
    if (current->proc->pid == pid)
    {
      current->pass += current->stride;
      return;
    }
    current = current->next;
  }
}

/**
 * find_smallest_pass_pid
 * Description: finds the process with the smallest pass value
 * Returns: pid_t - the process id of the process with the smallest pass value
 */
pid_t find_smallest_pass_pid()
{
  struct proc_list *current = head;
  struct proc_list *shortest = NULL;
  while (current != NULL)
  {
    if (current->proc->state == READY && (shortest == NULL || current->pass < shortest->pass || (current->pass == shortest->pass && current->proc->pid < shortest->proc->pid)))
    {
      shortest = current;
    }
    current = current->next;
  }
  return shortest ? shortest->proc->pid : -1;
}

/**
 * free_list
 * Description: frees all nodes in the process list and resets the head to NULL.
 * Returns: void
 */
void free_list()
{
  struct proc_list *current = head;
  while (current != NULL)
  {
    struct proc_list *temp = current;
    current = current->next;
    free(temp);
  }
  head = NULL;
}

/**
 * print_proc_list
 * Description: prints the process list
 * Returns: void
 */
void print_proc_list()
{
  struct proc_list *current = head;
  printf("Process Queue:\n");
  while (current != NULL)
  {
    printf("PID: %d, Stride: %d, Pass: %d, State: %d\n", current->proc->pid, current->stride, current->pass, current->proc->state);
    current = current->next;
  }
}

/* sched_init
 *   will be called exactly once before any processes arrive or any other events
 */
void sched_init()
{
  use_time_slice(TRUE);
}

/* sched_new_process
 *   will be called when a new process arrives (i.e., fork())
 *
 * proc - the new process that just arrived
 */
void sched_new_process(const struct process *proc)
{
  assert(READY == proc->state);
  int stride = STRIDE_CONSTANT / proc->tickets;

  if (proc_list_empty())
  {
    add_process((struct process *)proc, stride, 0);
    increment_pass(proc->pid);
    context_switch(proc->pid);
    return;
  }
  add_process((struct process *)proc, stride, 0);
}

/* sched_finished_time_slice
 *   will be called when the currently running process finished a time slice
 *   (This is only called when the time slice ends with time remaining in the
 *   current CPU burst.  If finishing the time slice happens at the same time
 *   that the process blocks / terminates,
 *   then sched_blocked() / sched_terminated() will be called instead).
 *
 * proc - the process whose time slice just ended
 *
 * Note: Time slice end events only occur if use_time_slice() is set to TRUE
 */
void sched_finished_time_slice(const struct process *proc)
{
  assert(READY == proc->state);
  run_new_process();
}

/* sched_blocked
 *   will be called when the currently running process blocks
 *   (e.g., if it starts an I/O operation that it needs to wait to finish
 *
 * proc - the process that just blocked
 */
void sched_blocked(const struct process *proc)
{
  assert(BLOCKED == proc->state);
  run_new_process();
}

/* sched_unblocked
 *   will be called when a blocked process unblocks
 *   (e.g., if its I/O operation finished)
 *
 * proc - the process that just unblocked
 */
void sched_unblocked(const struct process *proc)
{
  assert(READY == proc->state);
  // print_proc_list();
  if (is_only_one_ready())
  {
    run_new_process();
  }
}

/* sched_terminated
 *   will be called when the currently running process terminates
 *   (i.e., it finished it's last CPU burst)
 *
 * proc - the process that just terminated
 *
 * Note: "kill" commands and other ways to terminate a process that is not
 *       currently running are not being simulated, so only the currently running
 *       process can actually terminate.
 */
void sched_terminated(const struct process *proc)
{
  assert(TERMINATED == proc->state);
  run_new_process();
}

/* sched_cleanup
 *   will be called exactly once after all processes have terminated and there
 *   are no more events left to occur, just before the simulation exits
 *
 * Note: Calling sched_cleanup() is guaranteed if the simulation has a normal exit
 *       but is not guaranteed in the case of fatal errors, crashes, or other
 *       abnormal exits.
 */
void sched_cleanup()
{
  free_list();
}

/**
 * run_new_process
 * Description: finds a new process to run and runs it, incrementing the pass
 *              of the process it runs.
 */
void run_new_process()
{
  // find next proc to run
  pid_t next_proc = find_smallest_pass_pid();
  if (next_proc == -1)
  {
    return;
  }
  increment_pass(next_proc);
  // context switch to next proc
  if (get_current_proc() != next_proc)
  {
    context_switch(next_proc);
  }
}
