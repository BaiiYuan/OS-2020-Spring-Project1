#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "process.h"

int assign_cpu(int pid, int core) {
    if (core > sizeof(cpu_set_t)) {
        fprintf(stderr, "Core index error.");
        return -1;
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);

    if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
        perror("sched_setaffinity");
        exit(1);
    }

    return 0;
}

int exec(Process proc) {
    int pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        long start_sec, start_nsec, end_sec, end_nsec;
        char to_dmesg[200];
        syscall(GET_TIME, &start_sec, &start_nsec);
        for (int cou = 0; cou < proc.t_exec; cou++) UNI_T(); // wait
        syscall(GET_TIME, &end_sec, &end_nsec);
        sprintf(to_dmesg, "[project1] %d %lu.%09lu %lu.%09lu\n", getpid(), start_sec, start_nsec, end_sec, end_nsec);
        syscall(LOG_INFO, to_dmesg);
        exit(0);
    }

    assign_cpu(pid, 1); // assgin to another cpu
    return pid;
}

int block(int pid) {
    struct sched_param param;
    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
    if (ret < 0) {
        perror("sched_setscheduler");
        return -1;
    }
    return ret;
}

int wakeup(int pid) {
    struct sched_param param;
    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
    if (ret < 0) {
        perror("sched_setscheduler");
        return -1;
    }
    return ret;
}

