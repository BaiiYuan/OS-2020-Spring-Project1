cur_proc = -1
prev_proc = -1
last_time = 0
total_time = 0
finish_n_proc = 0
ready_queue = []


def is_process_ready(proc):
    if proc["pid"] == -1 or proc["exec_time"] == 0 :
        return 0
    else:
        return 1

def get_next_process(policy, n_proc, proc):
    global cur_proc, prev_proc, last_time, total_time, finish_n_proc, ready_queue
    ret = -1
    if policy == "FIFO":
        if cur_proc != -1:
            return cur_proc
        for i in range(n_proc):
            if is_process_ready(proc[i]):
                return i
        return -1
    elif policy == "RR":
        if cur_proc == -1 or (total_time - last_time) % 500 == 0:
            if len(ready_queue) == 0:
                return cur_proc
            else:
                ret = ready_queue[0]
                if cur_proc != -1:
                    ready_queue.append(cur_proc)
                ready_queue = ready_queue[1:]
                return ret
        return cur_proc
    elif policy == "SJF":
        if cur_proc != -1:
            return cur_proc
        ret = -1
        for i in range(n_proc):
            if not is_process_ready(proc[i]):
                continue
            if ret == -1 or proc[i]["exec_time"] < proc[ret]["exec_time"]:
                ret = i
        return ret
    elif policy == "PSJF":
        ret = -1
        for i in range(n_proc):
            if not is_process_ready(proc[i]):
                continue
            if ret == -1 or proc[i]["exec_time"] < proc[ret]["exec_time"]:
                ret = i
        return ret
    return -1


def scheduling(policy, n_proc, proc):
    global cur_proc, prev_proc, last_time, total_time, finish_n_proc, ready_queue

    cur_proc = -1
    prev_proc = -1
    last_time = 0
    total_time = 0
    finish_n_proc = 0
    ready_queue = []

    record = [[-1, -1] for _ in range(n_proc)]
    while True:
        # print(f"{total_time}\r", end="")
        # Check finished process
        if cur_proc != -1 and proc[cur_proc]["exec_time"] == 0:
            record[cur_proc][1] = total_time
            prev_proc = cur_proc
            cur_proc = -1
            finish_n_proc += 1
            if finish_n_proc == n_proc:
                break

        # Check processes are ready or not
        for i in range(n_proc):
            if proc[i]["ready_time"] == total_time:
                proc[i]["pid"] = 666
                ready_queue.append(i)

        # Find next running process
        next_proc = get_next_process(policy, n_proc, proc)
        if next_proc != -1 and next_proc != cur_proc:
            if record[next_proc][0] == -1:
                record[next_proc][0] = total_time
            cur_proc = next_proc
            last_time = total_time

        # Run unit of time
        total_time += 1
        if cur_proc != -1:
            proc[cur_proc]["exec_time"] -= 1
    return record

for p in ["FIFO", "RR", "SJF", "PSJF"]:
    for i in range(1, 6):
        procs = []

        f_in = open(f"OS_PJ1_Test/{p}_{i}.txt")
        tmp = f_in.read().strip().split("\n")
        policy, N, tmp = tmp[0], int(tmp[1]), tmp[2:]
        for item in tmp:
            name, ready_time, exec_time = item.split()
            procs.append({
                    "name": name,
                    "ready_time": int(ready_time),
                    "exec_time": int(exec_time),
                    "pid": -1
                })

        records = scheduling(policy, N, procs)

        print(f"---------- {policy} {i} ----------")
        for c, record in enumerate(records):
            print(f'{procs[c]["name"]} -> start from {record[0]} to {record[1]}')
