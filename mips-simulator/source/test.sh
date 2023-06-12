asm_file=../Example_test_cases/$1/$1.asm
code_file=../Example_test_cases/$1/$1.txt
chpt_file=../Example_test_cases/$1/$1_checkpts.txt
in_file=../Example_test_cases/$1/$1.in
out_file=$1.out
correct_out=../Example_test_cases/$1/$1_correct.out
correct_dump_dir=../Example_test_cases/$1/correct_dump

make

./simulator $asm_file $code_file $chpt_file $in_file $out_file

echo "checking output file"
cmp $out_file $correct_out

if [ "$#" -gt "1" ]; then
    args=("$@")
    chpts=("${args[@]:1}")
    for i in "${chpts[@]}"; do
        echo "checking checkpoint at $i"
        cmp memory_$i.bin $correct_dump_dir/memory_$i.bin
        cmp register_$i.bin $correct_dump_dir/register_$i.bin
    done
fi