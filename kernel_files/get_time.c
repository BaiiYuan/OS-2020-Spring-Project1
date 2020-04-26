#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
asmlinkage void sys_get_time(long *sec, long *nsec) {
    struct timespec now;
    getnstimeofday(&now);
    *sec = now.tv_sec;
    *nsec = now.tv_nsec;
    printk("sec:%ld, nsec:%ld\n", now.tv_sec, now.tv_nsec);
}
