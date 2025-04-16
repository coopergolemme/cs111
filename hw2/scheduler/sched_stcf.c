#include "scheduler.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/*******************************************************
 * SHORTEST TIME TO COMPLETION FIRST (STCF) Scheduler  *
 * (also known as Shortest Remaining Time First (SRTF) *
 *******************************************************/

/*
 * A pointer to the running process, initialized to NULL
 */
static struct process *running_process = NULL;

struct process *run_new_process_return_old();

/*
 * proc_list
 *    A linked list of processes
 */
struct proc_list
{
  struct process *proc;
  struct proc_list *next;
};

static struct proc_list *head = NULL;

/**
 * add_process
 * Description: takes in a process and adds it to the process list. If the
 *              passed process is in the TERMINATED state, it will not be added.
 * Parameters: struct process *proc - the process to add
 * Returns: void
 */
void add_process(struct process *proc)
{
  if (proc->state == TERMINATED)
  {
    return;
  }
  struct proc_list *new_node = (struct proc_list *)malloc(sizeof(struct proc_list));
  if (new_node == NULL)
  {
    perror("Failed to allocate memory for new process node");
    // exit(EXIT_FAILURE);
  }
  new_node->proc = proc;
  new_node->next = head;
  head = new_node;
}
/**
 * remove_process
 * Description: removes a process from the process list
 * Parameters: pid_t pid - the process id of the process to remove
 * Returns: struct process * - the process that was removed
 */
struct process *remove_process(pid_t pid)
{
  struct proc_list *current = head;
  struct proc_list *previous = NULL;

  while (current != NULL && current->proc->pid != pid)
  {
    previous = current;
    current = current->next;
  }

  if (current == NULL)
  {
    return NULL;
  }

  if (previous == NULL)
  {
    head = current->next;
  }
  else
  {
    previous->next = current->next;
  }

  struct process *proc = current->proc;
  free(current);
  return proc;
}

/**
 * find_STC_process_pid
 * Description: finds the process with the shortest time to completion. If
 *              there is a tie, the process with the lowest pid is chosen.
 * Returns: pid_t pid- the process id of the process with the shortest time to
 *                  completion
 */
pid_t find_STC_process_pid()
{
  struct proc_list *current = head;
  struct proc_list *shortest = NULL;

  while (current != NULL)
  {
    if (current->proc->state == READY)
    {
      if (shortest == NULL ||
          current->proc->current_burst->remaining_time < shortest->proc->current_burst->remaining_time ||
          (current->proc->current_burst->remaining_time == shortest->proc->current_burst->remaining_time && current->proc->pid < shortest->proc->pid))
      {
        shortest = current;
      }
    }
    current = current->next;
  }

  if (shortest == NULL)
  {
    return -1; // No ready process found
  }

  return shortest->proc->pid;
}

/**
 * remove_STC_process
 * Description: removes the process with the shortest time to completion. If
 *              there is a tie, the process with the lowest pid is chosen.
 * Returns: struct process * - the process that was removed
 */
struct process *remove_STC_process()
{
  pid_t stc_pid = find_STC_process_pid();
  if (stc_pid == -1)
    return NULL;
  return remove_process(stc_pid);
}

/**
 * print_process_queue
 * Description: prints the process queue
 * Returns: void
 */
void print_process_queue()
{
  struct proc_list *current = head;
  printf("Process Queue:\n");
  while (current != NULL)
  {
    printf("PID: %d, Remaining Time: %d, State: %d\n", current->proc->pid, current->proc->current_burst->remaining_time, current->proc->state);
    current = current->next;
  }
}

/* sched_init
 *   will be called exactly once before any processes arrive or any other events
 */
void sched_init()
{
  use_time_slice(FALSE);
}

/* sched_new_process
 *   will be called when a new process arrives (i.e., fork())
 *
 * proc - the new process that just arrived
 */
void sched_new_process(const struct process *proc)
{
  assert(READY == proc->state);
  pid_t runningProcessId = get_current_proc();

  // If CPU is idle, context switch to the new process and don't enqueue it
  if (runningProcessId == -1)
  {
    running_process = (struct process *)proc;
    context_switch(proc->pid);
    return;
  }

  add_process(running_process);
  add_process((struct process *)proc);
  run_new_process_return_old();
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
  // When a process gets blocked by IO, we want to find the next shortest time to completion task, that is ready. If none are ready then we are idle, and just return.
  if (!(running_process->pid == proc->pid))
    return;

  // Consider the running process in scheduling new process
  add_process(running_process);
  run_new_process_return_old();
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
  if (get_current_proc() == -1)
  {
    run_new_process_return_old();
  }
  else
  {
    add_process(running_process);
    run_new_process_return_old();
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
  if (!(running_process->pid == proc->pid))
    return;
  run_new_process_return_old();
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
}

/* run_new_process
 * Looks for a STC process to run from the list of processes, and switches if there is
 * returns the old running process
 */
struct process *run_new_process_return_old()
{
  struct process *stc_proc = remove_STC_process();

  // idle state
  if (stc_proc == NULL)
  {
    return NULL;
  }
  // if the process is the same as the running process, return the running process
  if (stc_proc->pid == get_current_proc())
  {
    return running_process;
  }
  context_switch(stc_proc->pid);
  struct process *old_running_process = running_process;
  running_process = stc_proc;
  return old_running_process;
}