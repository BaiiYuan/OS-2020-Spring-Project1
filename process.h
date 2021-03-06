#include <sys/types.h>

#define LOG_INFO    334
#define GET_TIME    335

#define _FIFO       1
#define _RR         2
#define _SJF        3
#define _PSJF       4

#define UNI_T()     { volatile unsigned long iii; for(iii=0;iii<1000000UL;iii++); }

#ifndef _PROCESS_H
#define _PROCESS_H

typedef struct process {
    char name[64];
    int ready_time, exec_time, is_start;
    pid_t pid;
    long start_sec, start_nsec, end_sec, end_nsec;
} Process;

typedef struct queue {
    struct queue *next;
    int value;
} Queue;

int assign_cpu(int pid, int core);
int exec(Process proc);
int block(int pid);
int wakeup(int pid);

#endif
