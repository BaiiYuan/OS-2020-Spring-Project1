mkdir output
policy=("FIFO" "RR" "SJF" "PSJF" )
for ((i=0; i < ${#policy[@]}; i++)); do
    for (( j = 1; j <= 5; j++ )); do
        filename=${policy[$i]}_${j}
        echo "Process OS_PJ1_Test/$filename.txt ..."
        sudo dmesg -c
        sudo ./main < "OS_PJ1_Test/$filename.txt" > "output/${filename}_stdout.txt"
        dmesg | grep Project1 > "output/${filename}_dmesg.txt"
    done
done
sudo dmesg -c
sudo ./main < "OS_PJ1_Test/TIME_MEASUREMENT.txt" > "output/TIME_MEASUREMENT_stdout.txt"
dmesg | grep Project1 > "output/TIME_MEASUREMENT_dmesg.txt"
