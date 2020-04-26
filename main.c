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
static int total_time;
static int last_time;
static int finish_n_proc;

Process *read_input(int *policy, int *n_proc);
void scheduling(int policy_id, int n_proc, Process *proc) ;

int main(int argc, char **argv) {
    int n_proc, policy_id;
    Process *proc_pool = read_input(&policy_id, &n_proc);
    for (int i = 0; i < n_proc; i++) {
        printf("%s %d %d\n", proc_pool[i].name, proc_pool[i].ready_time, proc_pool[i].exec_time);
    }
    scheduling(policy_id, n_proc, proc_pool);
    return 0;
}

int get_next_process(int policy_id, int n_proc, Process *proc) {
    switch(policy_id) {
        case _FIFO:
            if (cur_proc != -1) { return cur_proc; }
            for(int i = 0; i < n_proc; i++) { // Directly find next in sorted proc
                if(proc[i].pid == -1 || proc[i].exec_time == 0) { continue; }
                return i;
            } break;
        case _RR:
            if ((total_time - last_time) % 500 == 0)  {
                int ret = (cur_proc + 1) % n_proc;
                while (proc[ret].pid == -1 || proc[ret].exec_time == 0) {
                    ret = (ret + 1) % n_proc;
                } return ret;
            } else if (cur_proc != -1) {
                return cur_proc;
            } else {
                for (int i = 0; i < n_proc; i++) {
                    if(proc[i].pid == -1 || proc[i].exec_time == 0) { continue; }
                    return i;
                }
            } break;
        case _SJF:
            if (cur_proc != -1) { return cur_proc; }
            // And then fall through PSJF
        case _PSJF:
            int ret = -1;
            for(int i = 0; i < n_proc; i++) { // Directly find next in sorted proc
                if(proc[i].pid == -1 || proc[i].exec_time == 0) { continue; }
                if (ret == -1 || proc[i].exec_time < proc[ret].exec_time) {
                    ret = i;
                }
            } return ret;
    }
    return -1;
}

void scheduling(int policy_id, int n_proc, Process *proc) {
    int sched_pid = getpid();
    assign_cpu(sched_pid, 0);
    int ret = wakeup(sched_pid);
    printf("pid: %d, ret: %i\n", sched_pid, ret);

    cur_proc = -1;
    total_time = 0;
    finish_n_proc = 0;

    while (1 == 1) {
        // Check finished process
        if (cur_proc != -1 && proc[cur_proc].exec_time == 0) {
            fprintf(stderr, "%s finish at time %d.\n", proc[cur_proc].name, total_time);
            waitpid(proc[cur_proc].pid, NULL, 0);
            cur_proc = -1;
            finish_n_proc++;

            if (finish_n_proc == n_proc) { break; } // End!
        }
        // Check processes are ready or not
        for (int i = 0; i < n_proc; i++) {
            if (proc[i].ready_time == total_time) {
                proc[i].pid = exec(proc[i]);
                block(proc[i].pid);
                fprintf(stderr, "%s ready at time %d.\n", proc[i].name, total_time);
            }
        }
        // Find next running process
        int next_proc = get_next_process(policy_id, n_proc, proc);
        if (next_proc != -1 && next_proc != cur_proc) {
            wakeup(proc[next_proc].pid);
            block(proc[cur_proc].pid);
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
    scanf("%s\n%d\n", sched_policy, n_proc);
    fprintf(stderr, "%s, %d\n", sched_policy, *n_proc);
    Process *proc = (Process*)malloc(*n_proc * sizeof(Process));
    for (int i = 0; i < *n_proc; i++) {
        scanf("%s %d %d\n", proc[i].name, &proc[i].ready_time, &proc[i].exec_time);
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
//     printf("Start:%ld-%ld, End:%ld-%ld\n", start_sec, start_nsec, end_sec, end_nsec);
// }
