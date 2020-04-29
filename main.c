#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"

static int cur_proc;
static int prev_proc;
static int total_time;
static int last_time;
static int finish_n_proc;
static Queue *ready_queue;

// #define DEBUG

Process *read_input(int *policy, int *n_proc);
void scheduling(int policy_id, int n_proc, Process *proc) ;

int main(int argc, char **argv) {
    int n_proc, policy_id;
    Process *proc_pool = read_input(&policy_id, &n_proc);

#ifdef DEBUG
    for (int i = 0; i < n_proc; i++) {
        fprintf(stderr, "%s %d %d\n", proc_pool[i].name, proc_pool[i].ready_time, proc_pool[i].exec_time);
    }
#endif

    scheduling(policy_id, n_proc, proc_pool);
    return 0;
}

int is_process_ready(Process proc) {
    if(proc.pid == -1 || proc.exec_time == 0) {
        return 0;
    } else {
        return 1;
    }
}

void enqueue(int value) {
    Queue *insert_point = ready_queue;
    while (1 == 1) {
        if (insert_point.next == NULL) {
            insert_point.next = (Queue*)malloc(sizeof(Queue));
            insert_point.next.value = value;
            break;
        } else {
            insert_point = insert_point->next;
        }
    }
}

int dequeue() {
    if (ready_queue == NULL) {
        return -1;
    } else {
        int tmp = ready_queue.value;
        ready_queue = ready_queue->next;
        return tmp;
    }
}

int get_next_process(int policy_id, int n_proc, Process *proc) {
    int ret = -1;
    switch(policy_id) {
        case _FIFO:
            if (cur_proc != -1) { return cur_proc; }
            for(int i = 0; i < n_proc; i++) { // Directly find next in sorted proc
                if(is_process_ready(proc[i])) {
                    return i;
                }
            } return -1;
        case _RR:
            if (cur_proc == -1 || (total_time - last_time) / 500 >= 1) {
                int next = dequeue();
                if (next == -1) {
                    return cur_proc;
                } else {
                    if (cur_proc != -1) {
                        enqueue(cur_proc);
                    } return next;
                }
            } return cur_proc;
        case _SJF:
            if (cur_proc != -1) { return cur_proc; }
            // And then fall through PSJF
        case _PSJF:
            ret = -1;
            for(int i = 0; i < n_proc; i++) { // Directly find next in sorted proc
                if(!is_process_ready(proc[i])) { continue; }

                if (ret == -1 || proc[i].exec_time < proc[ret].exec_time) {
                    ret = i;
                }
            } return ret;
    } return -1;
}

void scheduling(int policy_id, int n_proc, Process *proc) {
    int sched_pid = getpid();
    assign_cpu(sched_pid, 0);
    int ret = wakeup(sched_pid);

    cur_proc = -1;
    prev_proc = -1;
    last_time = 0;
    total_time = 0;
    finish_n_proc = 0;

    while (1 == 1) {
        // Check finished process
        if (cur_proc != -1 && proc[cur_proc].exec_time == 0) {
            waitpid(proc[cur_proc].pid, NULL, 0);
            syscall(GET_TIME, &proc[cur_proc].end_sec, &proc[cur_proc].end_nsec);

            char to_dmesg[200];
            sprintf(to_dmesg, "[Project1] %d %lu.%09lu %lu.%09lu\n", proc[cur_proc].pid,
                    proc[cur_proc].start_sec, proc[cur_proc].start_nsec,
                    proc[cur_proc].end_sec, proc[cur_proc].end_nsec);
            syscall(LOG_INFO, to_dmesg);

#ifdef DEBUG
            fprintf(stderr, "%s(%d) finish at time %d.\n", proc[cur_proc].name, cur_proc, total_time);
#endif

            prev_proc = cur_proc;
            cur_proc = -1;
            finish_n_proc++;

            if (finish_n_proc == n_proc) { break; } // End!
        }
        // Check processes are ready or not
        for (int i = 0; i < n_proc; i++) {
            if (proc[i].ready_time == total_time) {
                proc[i].pid = exec(proc[i]);
                block(proc[i].pid);
                enqueue(i);

                fprintf(stdout, "%s %d\n", proc[i].name, proc[i].pid);
                fflush(stdout);
                syscall(GET_TIME, &proc[i].start_sec, &proc[i].start_nsec);
#ifdef DEBUG
                fprintf(stderr, "%s(%d) ready at time %d.\n", proc[i].name, i, total_time);
#endif
            }
        }
        // Find next running process
        int next_proc = get_next_process(policy_id, n_proc, proc);
        if (next_proc != -1 && next_proc != cur_proc) {
            wakeup(proc[next_proc].pid);
            block(proc[cur_proc].pid);

#ifdef DEBUG
            fprintf(stderr, "Process context switch from %s(%d) to %s(%d) at time %d.\n",
                    proc[cur_proc].name, cur_proc,
                    proc[next_proc].name, next_proc, total_time);
#endif

            cur_proc = next_proc;
            last_time = total_time;
        }
        // Run unit of time
        UNI_T();
        total_time ++;
        if (cur_proc != -1) { proc[cur_proc].exec_time -= 1; }
    }
}

int cmp_proc(const void *a,const void *b){
    return ((Process *)a)->ready_time - ((Process *)b)->ready_time;
}

Process *read_input(int *policy, int *n_proc) {
    char sched_policy[16];
    fscanf(stdin, "%s\n%d\n", sched_policy, n_proc);
#ifdef DEBUG
    fprintf(stderr, "%s, %d\n", sched_policy, *n_proc);
#endif
    Process *proc = (Process*)malloc(*n_proc * sizeof(Process));
    for (int i = 0; i < *n_proc; i++) {
        fscanf(stdin, "%s %d %d\n", proc[i].name, &proc[i].ready_time, &proc[i].exec_time);
        proc[i].pid = -1;
    }
    qsort(proc, *n_proc, sizeof(Process), cmp_proc);

    if (strcmp(sched_policy, "FIFO") == 0) { *policy = _FIFO; }
    else if (strcmp(sched_policy, "RR") == 0) { *policy = _RR; }
    else if (strcmp(sched_policy, "SJF") == 0) { *policy = _SJF; }
    else if (strcmp(sched_policy, "PSJF") == 0) { *policy = _PSJF; }
    else {
        fprintf(stderr, "Invalid policy: %s", sched_policy);
        exit(0);
    }
    return proc;
}

// void test_sys_call() {
//     syscall(LOG_INFO, "GGininder\n");
//     long start_sec, start_nsec, end_sec, end_nsec;
//     syscall(GET_TIME, &start_sec, &start_nsec);
//     UNI_T();
//     syscall(GET_TIME, &end_sec, &end_nsec);
//     fprintf(stderr, "Start:%ld-%ld, End:%ld-%ld\n", start_sec, start_nsec, end_sec, end_nsec);
// }
