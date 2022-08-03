#include "scheduler.h"

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "file_format.h"
#include "workload.h"

/* Globals */

/* Scheduler state information */
struct scheduler_state g_sstate = { 0 };

/* Whether an interrupt has been received */
bool g_interrupt = false;

/* How big the simulated "workload" for the tasks should be. We calibrate our
 * workload based on rough CPU performance estimates. That way the code should
 * run in *roughly* the same amount of time regardless of hardware. */
unsigned long g_workload_clocks = 0;

/* Context of the main function: */
ucontext_t g_main_ctx;

/* Function pointer to the scheduling algorithm implementation: */
void (*g_scheduling_algo)(struct scheduler_state *sched_state) = NULL;
void swap(struct process_ctl_block *first, struct process_ctl_block *second);
/* Constants for printing out the process status bars */
static const char *hashes = "####################";
static const char *spaces = "                    ";


/**
 * Performs a user-level context switch. This allows us to have multiple
 * "threads of execution" in our programs without actually using multiple OS
 * processes/threads.
 *
 * Input:
 * - pid: The process id, which is an index into the g_sstate.pcbs array of
 *   process control blocks.
 */
void context_switch(int pid)
{
    /* Ensure that we don't try to switch to a process that doesn't exist: */
    assert(pid >= 0);
    assert(pid < g_sstate.num_processes);

    struct process_ctl_block *pcb = &g_sstate.pcbs[pid];
    struct process_ctl_block *old_pcb = current_pcb();

    /* Reset our alarm "interrupt" to fire again in 1 second: */
    alarm(1);

    if (pcb == old_pcb && g_sstate.current_quantum > 0) {
        /* We are context switching to the same process. Simply set its state
         * back to running and return; this will jump us back to the process
         * execution context. */
        pcb->state = RUNNING;
        g_sstate.current_quantum++;
        return;
    }

    ucontext_t *old_ctx = &old_pcb->context;
    if (g_sstate.current_quantum == 0 || old_pcb->state == TERMINATED) {
        /* Handle cases where there is no previous or "old" context to go back
         * to. This happens for the first run-through, as well as any time a job
         * terminates and ends up down in the main() loop. */
        old_ctx = &g_main_ctx;
    }

    pcb->state = RUNNING;
    /* Update global state variables: */
    g_sstate.current_process = pid;
    g_sstate.current_quantum++;

    /* Do the context switch with the swapcontext() function: */
    printf("[i] Context switch: PID %d -> PID %d\n", old_pcb->pid, pid);
    int result = swapcontext(old_ctx, &pcb->context);
    if (result == -1) {
        perror("swapcontext");
    }
    return;
}

/**
 * Utility function to retrieve the current PCB.
 */
struct process_ctl_block *current_pcb(void)
{
    return &g_sstate.pcbs[g_sstate.current_process];
}

/**
 * Iterates through the list of process control blocks and checks for "arriving"
 * processes. Since we load the entire list of process executions at the start
 * of the program, we're just checking to see if a given process was supposed to
 * be created during the current quantum. If it was, we move it to the waiting
 * state and create its execution context.
 */
void handle_arrivals(void)
{
    int i;
    for (i = 0; i < g_sstate.num_processes; ++i) {
        if (g_sstate.pcbs[i].creation_quantum == g_sstate.current_quantum) {
            struct process_ctl_block *pcb = &g_sstate.pcbs[i];
            pcb->state = WAITING;
            pcb->arrival_time = get_time();

            getcontext(&pcb->context);
            pcb->context.uc_stack.ss_sp = pcb->stack;
            pcb->context.uc_stack.ss_size = sizeof(pcb->stack);
            pcb->context.uc_link = &g_main_ctx;
            makecontext(&pcb->context, process, 0);
            printf("[*] New process arrival: %s\n", pcb->name);
        }
    }
}

/**
 * We use a SIGALRM signal to emulate our interrupt. The process works like
 * this:
 * - We set an alarm to fire in one second: alarm(1)
 * - The alarm occurs, calling this signal handler
 * - We set g_interrupt to true to cause the currently-running process to stop
 *   and call interrupt_handler().
 *
 * Input:
 *  - signo: the signal number being handled. Should be SIGALRM.
 */
void signal_handler(int signo)
{
    assert(signo == SIGALRM);
    g_interrupt = true;
}

/**
 * Upon receipt of an interrupt, this function updates the current process
 * state, handles any new process arrivals, and then calls the scheduling logic
 * (a function pointed to by g_scheduling_algo).
 */
void interrupt_handler(void)
{
    g_interrupt = false;
    printf(" -> interrupt (%d)\n", g_sstate.current_quantum);

    /* The process was interrupted, so we should change its state back to
     * waiting. */
    if (current_pcb()->state == RUNNING) {
        current_pcb()->state = WAITING;
    }

    handle_arrivals();

    g_scheduling_algo(&g_sstate);
}

/**
 * Simulates process execution and prints progress.
 */
void process(void)
{
    struct process_ctl_block *pcb = current_pcb();
    pcb->start_time = get_time();
    printf("[>] PID %d starting ('%s', workload: %d)\n",
            pcb->pid, pcb->name, pcb->workload);

    while (pcb->executed_work < pcb->workload) {
        int i;
        for (i = 0; i < 20; ++i) {
            RUN_WORKLOAD(g_workload_clocks / 20);
            int perc = 100 * ((float) pcb->executed_work / pcb->workload);
            printf("\r%s [%s%s] %3d%%",
                    pcb->name,
                    &hashes[20 - perc / 5],
                    &spaces[perc / 5], perc);
            fflush(stdout);
            if (g_interrupt) {
                interrupt_handler();
            }
        }
        pcb->executed_work++;
    }

    int perc = 100;
    printf("\r%s [%s%s] %3d%%",
            pcb->name,
            &hashes[20 - perc / 5],
            &spaces[perc / 5], perc);
    pcb->state = TERMINATED;
    pcb->completion_time = get_time();
    printf(" -> terminated");
}

/**
 * A basic scheduler that simply runs each process in the array of PCBs based on
 * its array index; i.e., index 0 runs first, followed by index 1, and so on.
 * This is about as far from a 'real' scheduler as you can get!
 */
void basic(struct scheduler_state *sched_state)
{
    int i;
    for (i = 0; i < sched_state->num_processes; ++i) {
        struct process_ctl_block *pcb = &sched_state->pcbs[i];
        if(pcb->state == WAITING) {
            context_switch(i);
            return;
        }
    }
}

/**
 *
 */
void sjf(struct scheduler_state *sched_state)
{
    /*No process is running yet*/
    if(sched_state -> current_quantum == 0)
    {
        int i, pid;
        unsigned int wkld = UINT_MAX;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                    pid = i;
                    wkld = pcb -> workload;
                }
            }
        }
        context_switch(pid);
    }
    /*the current process has been interrupted*/
    else if(sched_state -> current_quantum > 0 && current_pcb() -> state == WAITING) return;
    /*the current process has terminated and we are looking for another process with the smallest workload*/
    else
    {
        int i, pid;
        unsigned int wkld = UINT_MAX;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                    pid = i;
                    wkld = pcb -> workload;
                }
            }
        }
        context_switch(pid);
    }
    /*Control loops dictates which process should be ran and after its done, it simple returns to the running process*/
    return;
}

/**
 *
 */
void psjf(struct scheduler_state *sched_state)
{    
    /*No process is running yet*/
    if(sched_state -> current_quantum == 0)
    {
        int i, pid;
        unsigned int wkld = UINT_MAX;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                    pid = i;
                    wkld = pcb -> workload;
                }
            }
        }
        context_switch(pid);
    }
    /*the current process has been interrupted*/
    else if(sched_state -> current_quantum > 0 && current_pcb() -> state == WAITING)
    {
        int i, pid;
        unsigned int wkld = current_pcb() -> workload;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                    pid = i;
                    wkld = pcb -> workload;
                }
            }
        }
        //(sched_state -> pcbs[sched_state -> current_process])
        if(wkld < current_pcb() -> workload) context_switch(pid);
        
    }   
    /*the current process has terminated and we are looking another process with the smallest workload*/
    else
    {
                int i, pid;
                unsigned int wkld = UINT_MAX;
                for (i = 0; i < sched_state->num_processes; ++i) 
                {
                    struct process_ctl_block *pcb = &sched_state->pcbs[i];
                    if(pcb->state == WAITING) 
                    {
                        if(pcb -> workload < wkld) 
                        {
                            pid = i;
                            wkld = pcb -> workload;
                        }
                    }
                }
                context_switch(pid);
    }
    
    return;
}

/**
 *
 */
void sctf(struct scheduler_state *sched_state)
{
    /*No process is running yet*/
    if(sched_state -> current_quantum == 0)
    {
        int i, pid;
        unsigned int wkld = UINT_MAX;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                    pid = i;
                    wkld = pcb -> workload;
                }
            }
        }
        context_switch(pid);
    }
    /*the current process has been interrupted*/
    else if(sched_state -> current_quantum > 0 && current_pcb() -> state == WAITING)
    {
        int i, pid = INT_MAX;
        unsigned int unfnwkld = current_pcb() -> workload - current_pcb() -> executed_work;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < unfnwkld) 
                {
                    pid = i;
                    unfnwkld = pcb -> workload;
                }
            }
        }
        if(pid != INT_MAX) context_switch(pid);
    }
    /*the current process has terminated and we are looking another process with the smallest workload*/
    else
    {
        int i, pid;
        unsigned int wkld = UINT_MAX;
        for (i = 0; i < sched_state->num_processes; ++i) 
        {
            struct process_ctl_block *pcb = &sched_state->pcbs[i];
            if(pcb->state == WAITING) 
            {
                if(pcb -> workload < wkld) 
                {
                     pid = i;
                     wkld = pcb -> workload;
                }
            }
        }
        context_switch(pid);
    }

    return;
}

void fifo(struct scheduler_state *sched_state)
{
    int i;
    int first = INT_MAX;
    int run = 0;

    if (sched_state -> current_quantum > 0 && current_pcb() -> state == WAITING ) {
        context_switch(sched_state->current_process);
        return;
    }

    for (i = 0; i < sched_state->num_processes; ++i) {
        struct process_ctl_block *pcb = &sched_state->pcbs[i];
        if(pcb->state == WAITING &&  pcb->creation_quantum < first) {
            first = pcb->creation_quantum;
            run = i;

        }
    }

    context_switch(run);
    return;
    
    
}

void RR(struct scheduler_state *sched_state)
{
    int i;
    unsigned int num = sched_state->current_process;
    for (i = 0; i < sched_state->num_processes; ++i) {
        
        num++;
        num = num % sched_state->num_processes;
        struct process_ctl_block *pcb = &sched_state->pcbs[num];
        if(pcb->state == WAITING ) {
            context_switch(num);
            return;
        }
    }
}

void Priority(struct scheduler_state *sched_state)
{
    int i;
    int priority = INT_MAX;
    int run = 0;

    
    for (i = 0; i < sched_state->num_processes; ++i) {
        struct process_ctl_block *pcb = &sched_state->pcbs[i];
        if(pcb->state == WAITING &&  pcb->priority < priority) {
            priority = pcb->priority;
            run = i;
        }
    }

    if (sched_state->pcbs[run].pid == current_pcb()->pid) {
        unsigned int num = sched_state->current_process;
        for (i = 0; i < sched_state->num_processes; ++i) {
            num++;
            num = num % sched_state->num_processes;
            struct process_ctl_block *pcb = &sched_state->pcbs[num];
            if(pcb->state == WAITING && pcb->priority == priority) {
                context_switch(num);
                return;
            }
        }
    }

    context_switch(run);
    return;
}

void insanity(struct scheduler_state *sched_state)
{
    int i;
    for (i = 0; i < sched_state->num_processes; ++i) {
        int n = rand() % sched_state->num_processes;
        struct process_ctl_block *pcb = &sched_state->pcbs[n];
        if(pcb->state == WAITING) {
            context_switch(n);
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    int seed = time(NULL);
    srand(seed);
    if (argc != 3) {
        printf("Usage: %s <process-spec>\n", argv[0]);
        return 1;
    }
    if (strcmp("fifo", argv[1]) == 0) {
        g_scheduling_algo = &fifo;
    }
    else if (strcmp("sjf", argv[1]) == 0) {
        g_scheduling_algo = &sjf;
    }
    else if (strcmp("psjf", argv[1]) == 0) {
        g_scheduling_algo = &psjf;
    }
    else if (strcmp("sctf", argv[1]) == 0) {
        g_scheduling_algo = &sctf;
    }
    else if (strcmp("rr", argv[1]) == 0) {
        g_scheduling_algo = &RR;
    }
    else if (strcmp("priority", argv[1]) == 0) {
        g_scheduling_algo = &Priority;
    }
    else if (strcmp("insanity", argv[1]) == 0) {
        g_scheduling_algo = &insanity;
    } 
    else {
        fprintf(stderr, "unknown command \n");
        return 1;
    }

    read_spec(argv[2], &g_sstate);

    g_workload_clocks = calibrate_workload();

    /* Set up our interrupt. Instead of a hardware interrupt, we'll be using
     * signals, a type of software interrupt. It will fire every 1 second. */
    signal(SIGALRM, signal_handler);


    printf("Starting execution ");
    getcontext(&g_main_ctx);
    bool finished = false;
    while (!finished) {
        interrupt_handler();
        if (g_sstate.num_processes > 0) {
            finished = true;
            int i;
            for (i = 0; i < g_sstate.num_processes; ++i) {
                if (g_sstate.pcbs[i].state != TERMINATED) {
                    finished = false;
                } 
            }
        }
    }

    printf("\n\nExecution complete.");
    printf(" Summary:\n");
    printf("---------------------------\n");
    double turnaround_time_total = 0;
    double response_time_total = 0;
    printf("Turnaround Times: \n");
    int i;

    for (i = 0; i < g_sstate.num_processes; ++i) {
        printf(" - %s %.2fs\n",g_sstate.pcbs[i].name, (g_sstate.pcbs[i].completion_time-g_sstate.pcbs[i].start_time));
        turnaround_time_total += (g_sstate.pcbs[i].completion_time-g_sstate.pcbs[i].start_time);
    }
    printf("\nAverage turnaround time: %.2fs\n", turnaround_time_total/g_sstate.num_processes);

    printf("\nResponse Times:\n");
    for (i = 0; i < g_sstate.num_processes; ++i) {
        printf(" - %s %.2fs\n",g_sstate.pcbs[i].name, (g_sstate.pcbs[i].start_time-g_sstate.pcbs[i].arrival_time));
        response_time_total += (g_sstate.pcbs[i].start_time-g_sstate.pcbs[i].arrival_time);
    }
    printf("\nAverage response time: %.2fs\n", response_time_total/g_sstate.num_processes);
    int j;
    int min;
    printf("\nCompletion Order:\n");
    for (i = 0; i < g_sstate.num_processes; ++i) {
        min = i; 
        for (j = i+1; j < g_sstate.num_processes; j++) {
          if (g_sstate.pcbs[j].completion_time < g_sstate.pcbs[min].completion_time) {
            min = j; 
          }
        }
        swap(&g_sstate.pcbs[min], &g_sstate.pcbs[i]);
        printf("%d.) %s\n", i, g_sstate.pcbs[i].name);
    }    
    return 0;
}

void swap(struct process_ctl_block *first, struct process_ctl_block *second) 
{ 
    struct process_ctl_block temp = *first; 
    *first = *second; 
    *second = temp; 
} 
