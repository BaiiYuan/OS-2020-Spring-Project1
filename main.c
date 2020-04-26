#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "process.h"

static int t_last;
static int t_total;
static int cur_proc;
static int finish_cnt;


int scheduling(int policy_id, int n_proc, Process *proc) {
    int sched_pid = getpid();
    assign_cpu(sched_pid, 0);
    int ret = wakeup(sched_pid); 
    printf("pid: %d, ret: %i\n", sched_pid, ret);
    
    while (true) {
        
        UNI_T();
        if (cur_proc != -1) proc[cur_proc].t_exex -= 1;
        t_total ++;
    }
    return 0;
}

int cmp_proc(const void *a,const void *b){
    return ((Process *)a)->t_ready - ((Process *)b)->t_ready;
}

int read_input(int *n_proc, Process *proc) {
    char sched_policy[16];
    scanf("%s\n%d\n", sched_policy, n_proc);
    fprintf(stderr, "%s, %d\n", sched_policy, *n_proc);
    proc = (Process*)malloc(*n_proc * sizeof(Process));
    for (int i = 0; i < *n_proc; i++) {
        scanf("%s %d %d\n", proc[i].name, &proc[i].t_ready, &proc[i].t_exec);
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

