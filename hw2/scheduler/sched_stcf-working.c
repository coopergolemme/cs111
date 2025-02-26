#include "scheduler.h"
#include <assert.h>

/*******************************************************
 * SHORTEST TIME TO COMPLETION FIRST (STCF) Scheduler  *
 * (also known as Shortest Remaining Time First (SRTF) *
 *******************************************************/

#include <stdlib.h>
#include <stdio.h>

struct process_queue
{
    struct process *proc;
    struct process_queue *next;
};

static struct process_queue *head = NULL;
static struct process *running_process = NULL;

void add_process(struct process *proc)
{
    // TODO: check for malloc failure
    struct process_queue *new_node = (struct process_queue *)malloc(sizeof(struct process_queue));
    new_node->proc = proc;
    new_node->next = NULL;

    if (head == NULL)
    {
        head = new_node;
    }
    else
    {
        struct process_queue *current = head;
        struct process_queue *previous = NULL;

        while (current != NULL && current->proc->current_burst->remaining_time <= proc->current_burst->remaining_time)
        {
            previous = current;
            current = current->next;
        }

        if (previous == NULL)
        {
            new_node->next = head;
            head = new_node;
        }
        else
        {
            previous->next = new_node;
            new_node->next = current;
        }
    }
}

struct process *remove_process(pid_t pid)
{
    struct process_queue *current = head;
    struct process_queue *previous = NULL;

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

pid_t find_STC_process_pid()
{
    struct process_queue *current = head;
    struct process_queue *shortest = NULL;

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

struct process *remove_STC_process()
{
    pid_t stc_pid = find_STC_process_pid();
    if (stc_pid == -1)
    {
        return NULL;
    }
    return remove_process(stc_pid);
}

int is_list_empty()
{
    return head == NULL;
}
void print_process_queue()
{
    struct process_queue *current = head;
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
    // print_process(proc);

    assert(READY == proc->state);

    pid_t runningProcessId = get_current_proc();

    // print_process(proc);
    // printf("Running Process PID: %d\n", runningProcessId);

    // If CPU is idle, context switch to the new process and don't enqueue it
    if (runningProcessId == -1 && is_list_empty())
    {
        running_process = (struct process *)proc;
        context_switch(proc->pid);
        return;
    }
    // print_process(running_process);
    if (running_process->state == TERMINATED)
    {
        add_process((struct process *)proc);
        // print_process_queue();
        struct process *stc_proc = remove_STC_process();
        running_process = stc_proc;
        context_switch(running_process->pid);
        return;
    }

    // If the CPU is not idle

    time_ticks_t remaining_time_new_process = proc->current_burst->remaining_time;
    time_ticks_t remaining_time_running_process = running_process->current_burst->remaining_time;

    // If the time remaining for the new process is less than the running one, we can usurp
    if (remaining_time_new_process < remaining_time_running_process || !(running_process->state == READY))
    {
        // printf("New process (PID: %d) has shorter time to completion than the currently running process (PID: %d)\n", proc->pid, running_process->pid);
        running_process = (struct process *)proc;
        context_switch(proc->pid);
        // print_process_queue();

        return;
    }

    // If new process is longer, just enqueue
    add_process((struct process *)proc);
    // print_process_queue();
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
    // TODO: implement this
    // printf("Finished time slice");
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
    // printf("Process with PID: %d is now BLOCKED\n", proc->pid);
    // TODO: implement this
    // When a process gets blocked by IO, we want to find the next shortest time to completion task, that is ready. If none are ready then we are idle, and just return.

    // print_process_queue();
    if (is_list_empty())
    {
        return;
    }

    // print_process_queue();
    if (!(running_process->pid == proc->pid))
    {
        return;
    }

    // remove_process(proc->pid);

    struct process *stc_proc = remove_STC_process();
    // print_process(stc_proc);
    // If no processes are ready were are in an idle state
    if (stc_proc == NULL)
    {
        return;
    }

    // We want to switch the running process, context switch and add the process to list
    add_process(running_process);
    running_process = stc_proc;
    context_switch(running_process->pid);
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
    // Remove process
    remove_process(proc->pid);

    // If at that point, the list is empoty
    if (is_list_empty())
    {
        running_process = (struct process *)proc;
        context_switch(running_process->pid);
        return;
    }
    // printf("Running Process PID: %d\n", running_process->pid);
    // print_process(running_process);
    // TODO: implement this
    // We assume that the current running proc is the stc proccess.
    // Check if the remaining time is less than the current process
    time_ticks_t remaining_time_running_proc = running_process->current_burst->remaining_time;
    time_ticks_t remaining_time_unblocked_proc = proc->current_burst->remaining_time;
    // Case where we need to usurp
    if (remaining_time_unblocked_proc < remaining_time_running_proc)
    {
        add_process(running_process);
        running_process = (struct process *)proc;
        context_switch(running_process->pid);
        return;
    }
    if (remaining_time_running_proc == remaining_time_unblocked_proc)
    {
        if (running_process->pid < proc->pid)
        {
            return;
        }
        else
        {
            add_process(running_process);
            running_process = (struct process *)proc;
            context_switch(running_process->pid);
            return;
        }
    }
    add_process((struct process *)proc);
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
    {
        return;
    }

    const struct process *stc_proc = remove_STC_process();
    // idle state
    if (stc_proc == NULL)
    {
        return;
    }

    // maybe need to free running process
    running_process = (struct process *)stc_proc;

    context_switch(running_process->pid);
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
    // TODO: implement this
}
