# From C Program to CPU Pipeline

This repository contains a list of projects that eventually transform a high-level C language program into extremely low-level machine code and a CPU pipeline that can execute machine codes.

## Simplified C Language Compiler

It is a compiler based on `flex` and `bison` for a simplified version of C language.
Just like `gcc` compiler, it takes in the source program and generates corresponding executable MIPS codes.
Check it out at at [simc-compiler-mips](./simc-compiler-mips/)

## MIPS Assembler

An assembler for MIPS instructions, which takes in MIPS source codes and generate corresponding machine codes. It is implemented with regular expressions for maximum flexibility and easiest debugging. Check it out at [mips-assembler](./mips-assembler/).

## MIPS Simulator

A MIPS simulator based on C++ for emulation and debugging.
It not only supports regular MIPS instructions, but also supports various system calls such as file I/O.
Check it out at [mips-simulator](./mips-simulator/).

## MIPS CPU Pipeline

A 5-stage CPU pipeline for MIPS, implemented with verilog.
A few dozens of MIPS instructions are supported and all hazards are handled.
Check it out at [mips-pipeline](./mips-pipeline/)
