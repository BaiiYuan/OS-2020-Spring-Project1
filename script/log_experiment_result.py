T_UNIT = 1363665.6302

def diff_time(a, b):
    a_sec, a_nsec = a.split(".")
    b_sec, b_nsec = b.split(".")
    a_sec, a_nsec = int(a_sec), int(a_nsec)
    b_sec, b_nsec = int(b_sec), int(b_nsec)
    return (a_sec - b_sec) * 1e9 + (a_nsec - b_nsec)

for p in ["FIFO", "RR", "SJF", "PSJF"]:
    for i in range(1, 6):
        name2time, name2pid, pid2_time = {}, {}, {}
        order = []

        f_in = open(f"OS_PJ1_Test/{p}_{i}.txt")
        tmp = f_in.read().strip().split("\n")
        policy, N, content = tmp[0], int(tmp[1]), tmp[2:]
        min_name, min_value = "", 9999999999
        for item in content:
            name, start_time, exec_time = item.split()
            order.append(name)
            name2time[name] = (int(start_time), int(exec_time))
            if int(start_time) < min_value:
                min_name = name
                min_value = int(start_time)
                min_exec = int(exec_time)
            elif int(start_time) == min_value and p in ["SJF", "PSJF"] and int(exec_time) < min_exec:
                min_name = name
                min_value = int(start_time)
                min_exec = int(exec_time)

        f_out = open(f"output/{p}_{i}_stdout.txt")
        content = f_out.read().strip().split("\n")
        for item in content:
            name, pid = item.split()
            name2pid[name] = pid

        f_dmes = open(f"output/{p}_{i}_dmesg.txt")
        content = f_dmes.read().strip().split("\n")
        for item in content:
            pid, start_time, end_time = item.split("[Project1]")[1].split()
            pid2_time[pid] = (start_time, end_time)

        base_time = pid2_time[name2pid[min_name]][0]

        f_dmes = open(f"output/{p}_{i}_dmesg.txt")
        content = f_dmes.read().strip().split("\n")
        for item in content:
            pid, start_time, end_time = item.split("[Project1]")[1].split()
            pid2_time[pid] = (int(diff_time(start_time, base_time) // T_UNIT + min_value),
                              int(diff_time(end_time, base_time) // T_UNIT + min_value))

        print(f"---------- {policy} {i} ----------")
        for name in order:
            start, end = pid2_time[name2pid[name]]
            print(f"{name} -> start from {start} to {end}")
