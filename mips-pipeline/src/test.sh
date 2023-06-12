for i in 1 2 3 4 5 6 7 8; do
    cp ../../testcase/cpu_test/machine_code${i}.txt ./CPU_instruction.bin
    make test
    echo "memory difference for testcase ${i}:"
    cmp ./data.bin ../../testcase/cpu_test/DATA_RAM${i}.txt
    sleep 1
done