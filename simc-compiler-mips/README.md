# A Compiler for Simplified C Grammar

## Environment

```
gcc
make
flex
bison
```

See the [docker file](./flex-bison.Dockerfile) for how to configure the environment.

## The Simplified C Language

### Tokens

```
# keywords
int {INT} main { MAIN } void { VOID } break { BREAK }
do { DO } else { ELSE } if { IF } while { WHILE }
return { RETURN } scanf { READ } printf { WRITE }

# special symbols
"{" {LBRACE} "}" {RBRACE} "[" {LSQUARE} "]" {RSQUARE}
"(" {LPAR} ")" {RPAR} ";" {SEMI} "+" {PLUS}
"−" {MINUS} "∗" {MUL OP} "/" {DIV OP} "&" {AND OP}
"|" {OR OP} "!" {NOT OP} "=" {ASSIGN} "<" {LT}
">" {GT} "<<" {SHL OP} ">>" {SHR OP} "==" {EQ}
"!=" {NOTEQ} "<=" {LTEQ} ">=" {GTEQ} "&&" {ANDAND}
"||" {OROR} "," {COMMA}

# numbers and variables
digit = [0−9]
letter = [a−zA−Z]
INT_NUM = [digit]+
ID = [letter]+[digit|letter|_]∗
```

### Grammar

```yacc
1. program: var declarations statements
2. var declarations : var declarations | var declaration | /∗ empty ∗/
3. var declaration : INT declaration list SEMI
4. declaration list : declaration list COMMA declaration | declaration
5. declaration : IDENTIFIER ASSIGN NUMBER | IDENTIFIER LSQUARE NUMBER RSQUARE | IDENTIFIER
6. code block: statement | LBRACE statements RBRACE
7. statements: statements statement | statement
8. statement: assign statement SEMI | control statement | read write statement SEMI | SEMI
9. control statement : if statement | while statement | do while statement SEMI | return statement SEMI
10. read write statement : read statement | write statement
11. assign statement : IDENTIFIER LSQUARE exp RSQUARE ASSIGN exp | IDENTIFIER ASSIGN exp
12. if statement : if stmt | if stmt ELSE code block
13. if stmt : IF LPAR exp RPAR code block
14. while statement : WHILE LPAR exp RPAR code block
15. do while statement: DO code block WHILE LPAR exp RPAR
16. return statement : RETURN
17. read statement: READ LPAR IDENTIFIER RPAR
18. write statement : WRITE LPAR exp RPAR
19. exp: INT NUM | IDENTIFIER | IDENTIFIER LSQUARE exp LSQUARE | NOT OP exp | exp AND OP exp | exp OR OP exp | exp PLUS exp | exp MINUS exp | exp MUL OP exp | exp DIV OP exp | exp LT exp | exp GT exp | exp EQ exp | exp NOTEQ exp | exp LTEQ exp | exp GTEQ exp | exp SHL OP exp | exp SHR OP exp | exp ANDAND exp | exp OROR exp | LPAR exp RPAR | MINUS exp
```

## How to Run

Compile:

```bash
cd source
make
```

Run:

```bash
cd source
./comp <input-file>  # compiled MIPS codes will be written to stdout
# or 
./comp <input-file> <output-file>
```

Examples:

```bash
./comp ../TestCases/test2.c1
```

```asm
.data
var: .byte 0 : 5
stack: .word 0: 2
.text
    la $16, var
    la $17, stack
    li $8, 3
    sw $8, 0($16)
    li $8, 3
    sw $8, 0($17)
    lw $8, 0($16)
    lw $9, 0($17)
    sub $8, $8, $9
    slti $8, $8, 1
    xori $8, $8, 1
    beqz $8, B_1_end
    li $8, 2
    sw $8, 0($17)
    lw $8, 0($16)
    lw $9, 0($17)
    sub $8, $8, $9
    slti $8, $8, 1
    xori $8, $8, 1
    beqz $8, B_0_end
    li $8, 1
    sw $8, 0($17)
    lw $8, 0($16)
    lw $9, 0($17)
    add $8, $8, $9
    sw $8, 0($16)
B_0_end:
    li $8, 1
    sw $8, 0($17)
    lw $8, 0($16)
    lw $9, 0($17)
    add $8, $8, $9
    sw $8, 0($16)
    j B_2_end
B_1_end:
    li $8, 1
    sw $8, 0($17)
    lw $8, 0($16)
    lw $9, 0($17)
    sub $8, $8, $9
    sw $8, 0($16)
B_2_end:
    lw $8, 0($16)
    move $4, $8
    li $2, 1
    syscall 
    li $2, 11
    li $4, 10
    syscall 
    li $2, 10
    syscall 
```

The output MIPS assemble file can be run (simulated) in [MARS simulator](http://courses.missouristate.edu/kenvollmar/mars/).