#include <linux/linkage.h>
#include <linux/kernel.h>
asmlinkage void sys_log_info(char* message) {
    printk("%s", message);
}
