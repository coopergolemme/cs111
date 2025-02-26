#include "scheduler.h"
#include "process.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
/*************************
 * ROUND ROBIN Scheduler *
 *************************/

/**
 * process_queue
 *   A queue of processes to be run
 */
struct process_queue
{
  struct process *proc;
  struct process_queue *next;
};

static struct process_queue *head = NULL;
static struct process_queue *tail = NULL;

/**
 * enqueue_process
 * Description: enqueues a process to the process queue
 * Parameters:
 *  proc - the process to enqueue
 * Returns: void
 */
void enqueue_process(struct process *proc)
{
  struct process_queue *new_node = (struct process_queue *)malloc(sizeof(struct process_queue));
  if (new_node == NULL)
  {
    perror("Failed to allocate memory for new process queue node");
    exit(EXIT_FAILURE);
  }
  new_node->proc = proc;
  new_node->next = NULL;
  if (tail == NULL)
  {
    head = tail = new_node;
  }
  else
  {
    tail->next = new_node;
    tail = new_node;
  }
}

/**
 * dequeue_process
 * Description: dequeues a process from the process queue
 * Returns: struct process * - the process that was dequeued
 */
struct process *dequeue_process()
{
  if (head == NULL)
  {
    return NULL;
  }
  struct process *proc = head->proc;
  struct process_queue *temp = head;
  head = head->next;
  if (head == NULL)
  {
    tail = NULL;
  }
  free(temp);
  return proc;
}

int is_queue_empty()
{
  return head == NULL;
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
  // Current running process
  pid_t runningProcessId = get_current_proc();

  // If the CPU is idle, context switch to the new process and don't enqueue it.
  if (is_queue_empty() && runningProcessId == -1)
  {
    context_switch(proc->pid);
    return;
  }
  // If the CPU is not idle, enqueue the new process.
  enqueue_process((struct process *)proc);
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

  // If the queue is empty, we reach an idle state. Just enqueue the process and return.
  if (is_queue_empty())
  {
    // This is a check for if there is only one process left to run
    if (get_current_proc() == proc->pid)
    {
      return;
    }
    enqueue_process((struct process *)proc);
    return;
  }
  // If the queue is not empty, dequeue a process and context switch to it, enqueuing the time-sliced process.
  const struct process *next_proc = dequeue_process();
  context_switch(next_proc->pid);
  enqueue_process((struct process *)proc);
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
  // If the queue is empty, we reach an idle state. Just return.
  if (is_queue_empty())
  {
    return;
  }
  // If the queue is not empty, dequeue a process and context switch to it.
  const struct process *next_proc = dequeue_process();
  context_switch(next_proc->pid);
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
  // If queue is empty, context switch to the recently unblocked process.
  if (is_queue_empty())
  {
    context_switch(proc->pid);
    return;
  }
  // If queue is not empty, enqueue the recently unblocked process.
  enqueue_process((struct process *)proc);
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
  // If the queue is empty, we reach an idle state. Just return.
  if (is_queue_empty())
  {
    return;
  }

  // If the queue is not empty, dequeue a process and context switch to it.
  const struct process *next_proc = dequeue_process();
  context_switch(next_proc->pid);
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
  while (!is_queue_empty())
  {
    struct process *proc = dequeue_process();
    free(proc);
  }
  head = NULL;
  tail = NULL;
}
