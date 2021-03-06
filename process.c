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

int assign_cpu(int pid, int core_id) {
    if (core_id > sizeof(cpu_set_t)) {
        perror("Core index error.");
        return -1;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);

    if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
        perror("sched_setaffinity");
        return -1;
    }
    return 0;
}

int exec(Process proc) {
    int pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    else if (pid == 0) { // child
        // Exec unit of time
        for (int cou = 0; cou < proc.exec_time; cou++) UNI_T();
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

