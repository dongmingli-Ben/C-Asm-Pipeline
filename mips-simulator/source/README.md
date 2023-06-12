# MIPS Simulator

In this directory, only the `.cpp` and `.h` files are needed for compilation. `.py`, `.json`, and `.sh` are used for easier development and testing.

## How to Run

First, compile the simulator:

```bash
make
```

Second, run the simulator:

```bash
./sim <asm-file> <code-file> <chpt-file> <in-file> <out-file>
```

Simulated output will be written to `<out-file>`.

## Testing

A shell script has been written for easier testing. It can be used as follows:

```bash
bash test.sh <test-case-name> <checkpoint-1> <checkpoint-2> ... <checkpoint-n>
```

For example, to test `a-plus-b` with given checkpoint 0, run

```bash
bash test.sh a-plus-b 0
```

You should see output in terminal like below:

```
checking output file
checking checkpoint at 0
```

MIPS output is in `a-plus-b.out`, which should be:

```
-101
```