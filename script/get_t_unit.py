f_dmes = open(f"output/TIME_MEASUREMENT_dmesg.txt")
content = f_dmes.read().strip().split("\n")
record = []
for item in content:
    pid, start_time, end_time = item.split("[Project1]")[1].split()
    duration = int(end_time.replace('.','')) - int(start_time.replace('.',''))
    record.append(duration)

print(f"All reocrds: {record}")
print(f"Unit Y time: {sum(record) / 10 / 500} (ns)")
