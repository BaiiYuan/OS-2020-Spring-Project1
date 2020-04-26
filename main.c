#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "process.h"

static int cur_proc;
static int t_total;
static int finish_n_proc;
static int t_last;

int set_next_process(int policy_id, int n_proc, Process *proc) {
    switch(policy_id) {
        case _FIFO:
            if (cur_proc != -1) return cur_proc;

            for(int i = 0; i < n_proc; i++) { // Directly find next in sorted proc
                if(proc[i].pid == -1 || proc[i].t_exec == 0) {
                    continue;
                }
                return i;
            }
            break;
        case _RR:
            break;
        case _SJF:
            if (cur_proc != -1) return cur_proc;
            break;
        case _PSJF:
            break;
    }
    return -1;
}


void scheduling(int policy_id, int n_proc, Process *proc) {
    int sched_pid = getpid();
    assign_cpu(sched_pid, 0);
    int ret = wakeup(sched_pid);
    printf("pid: %d, ret: %i\n", sched_pid, ret);

    cur_proc = -1;
    t_total = 0;
    finish_n_proc = 0;

    while (true) {
        fprintf(stderr, "Current time: %d\n", t_total);

        if (cur_proc != -1 && proc[cur_proc].exec_time == 0) {
            waitpid(proc[cur_proc].pid, NULL, 0);
            printf("%s %d\n", proc[cur_proc].name, proc[cur_proc].pid);
            cur_proc = -1;
            finish_n_proc++;

            if (finish_n_proc == nproc) break; // End!
        }

        // Check the process is ready or not
        for (int i = 0; i < n_proc; i++) {
            if (proc[i].ready_time == ntime) {
                proc[i].pid = exec(proc[i]);
                block(proc[i].pid);
                fprintf(stderr, "%s ready at time %d.\n", proc[i].name, ntime);
            }
        }

        // Find next running process
        int next_proc = next_process(proc, nproc, policy);
        if (next_proc != -1 && next_proc != cur_proc) {
            proc_wakeup(proc[next_proc].pid);
            proc_block(proc[cur_proc].pid);
            cur_proc = next_proc;
            t_last = t_total;
        }


        // Run unit of time
        UNI_T();
        if (cur_proc != -1) proc[cur_proc].t_exex -= 1;
        t_total ++;
    }
}

int cmp_proc(const void *a,const void *b){
    return ((Process *)a)->ready_time - ((Process *)b)->ready_time;
}

int read_input(int *n_proc, Process *proc) {
    char sched_policy[16];
    scanf("%s\n%d\n", sched_policy, n_proc);
    fprintf(stderr, "%s, %d\n", sched_policy, *n_proc);

    proc = (Process*)malloc(*n_proc * sizeof(Process));
    for (int i = 0; i < *n_proc; i++) {
        scanf("%s %d %d\n", proc[i].name, &proc[i].ready_time, &proc[i].exec_time);
        proc[i].pid = -1;
    }
    qsort(proc, *n_proc, sizeof(Process), cmp_proc);

    if (strcmp(sched_policy, "FIFO") == 0) return _FIFO;
    else if (strcmp(sched_policy, "RR") == 0) return _RR;
    else if (strcmp(sched_policy, "SJF") == 0) return _SJF;
    else if (strcmp(sched_policy, "PSJF") == 0) return _PSJF;
    else {
        fprintf(stderr, "Invalid policy: %s", sched_policy);
        exit(0);
    }
}

void test_sys_call() {
    syscall(LOG_INFO, "GGininder");
    long start_sec, start_nsec, end_sec, end_nsec;
    syscall(GET_TIME, &start_sec, &start_nsec);
    UNI_T();
    syscall(GET_TIME, &end_sec, &end_nsec);
    printf("Start:%ld-%ld, End:%ld-%ld\n", start_sec, start_nsec, end_sec, end_nsec);
}

int main(int argc, char **argv) {
    int n_proc;
    Process *proc_pool;
    int policy_id = read_input(&n_proc, proc_pool);
    test_sys_call();

    scheduling(policy_id, n_proc, proc_pool);
    return 0;
}

