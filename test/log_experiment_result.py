from IPython import embed

T_UNIT = 1339411.1138

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

        f_out = open(f"output/{p}_{i}_stdout.txt")
        content = f_out.read().strip().split("\n")
        for item in content:
            name, pid = item.split()
            name2pid[name] = pid

        f_dmes = open(f"output/{p}_{i}_dmesg.txt")
        content = f_dmes.read().strip().split("\n")
        for item in content:
            pid, start_time, end_time = item.split("[Project1]")[1].split()
            pid2_time[pid] = (int(start_time.replace('.','')), int(end_time.replace('.','')))

        base_time = pid2_time[name2pid[min_name]][0] - min_value * T_UNIT

        f_dmes = open(f"output/{p}_{i}_dmesg.txt")
        content = f_dmes.read().strip().split("\n")
        for item in content:
            pid, start_time, end_time = item.split("[Project1]")[1].split()
            pid2_time[pid] = (int((int(start_time.replace('.','')) - base_time) // T_UNIT),
                              int((int(end_time.replace('.','')) - base_time) // T_UNIT))

        print(f"---------- {policy} {i} ----------")
        for name in order:
            start, end = pid2_time[name2pid[name]]
            print(f"{name} -> start from {start} to {end}")
