# Simplified C Compiler

## Scanner

The scannar is implemented with `flex`. In brief, scanner can be implemented with deterministic finite autometa. The overall procedure is: regular expressions ==> nondeterministic finite autometa ==> deterministic finite autometa ==> scanner.

## Parser

The parser is implemented with `bison`, which constructs the parsing table, manages semantic records, and calls semantic routines to generate MIPS code.

The LR(1) grammar is constructed from assignment 3.

## CodeGen

### Semantic Representation

In general, statements (of the simplified C language) are in 2 categories, i.e. variable declarations (declare memory usage) and operations (operate on registers and memory).

Two kinds of semantic representations are used for these two kinds of statements. `declaration_struct` stores the variable's name and address in memory. `temp_value_struct` stores the code block for the statement.

Here are the details for the two structs:

```c
/*
A temporay value is a complete section.
One can obtain the temporary value with
global variables only.
*/
struct temp_value_struct {
    int register_num;  /* number of registers used in calculation */
    /* mips_code contains all necessary code to obtain the
       result and store it in $t0 */
    struct code_list *mips_code;
    /* stack size required to calculate the experssion */
    int stack_size;
};

/*
 * Singly linked list of symbols (in declarations)
 */
struct symbol {
    char name[33];
    int offset;    /* this field is invalid until register_symbol_table is performed */
    int initial_value;
    int length;    /* number of ints (for array) */
    int initialized; /* whether a initial value is set for this symbol */
    struct symbol *next;
};

/*
 * Struct for declarations.
 */
struct declaration_struct {
    struct symbol *head, *tail;
};
```

### Variable Declaration

When the variable declaration section is reduced, variables stored in `declaration_struct` will be registered to a global symbol table. Later operation statements will consult the global symbol table for the address of variables.

### Ordinary Operations

By ordinary, I mean "no control flow". The main issue to solve is to manage the stack so that operations are carried out successfully.

The strategy is that for operations that needs more than 1 input values (in simplified C language, each operation use at most 2 input values):

1. execute the codes for the first input value (obtain from `temp_value_struct` of the first input)

2. push the value to a local stack (value available at `$t0`)

3. execute the codes for the second input value (after completion, the value is at `$t0`)

4. pop the first value from local stack and place it to `$t1`

### Control Flow

In my implementation, control flows are managed using branch instructions and labels.

#### `if`

For a if statement with the following structure:

```yacc
IF LPAR exp RPAR code_block
```

My strategy is:

```bash
exp.codes  # output is at $t0
beqz $t0, if_end  # if it is false, jump to the end of the if statement
code_block.codes  # if true, execute the code blocks
if_end:   # label for the end of if statement
```

#### `if-else`

For a if statement with the following structure:

```yacc
if_stmt ELSE code_block
```

My strategy is:

```bash
exp.codes  # codes from if_stmt
beqz $t0, if_end  # codes from if_stmt
code_block.codes  # codes from if_stmt
j else_end  # jump to the end of if-else statement
if_end:   # codes from if_stmt
code_block.codes  # execute else branch
else_end: # label for the end  of if-else statement
```

#### `do-while`

For a do-while statement with the following structure:

```yacc
DO code_block WHILE LPAR exp RPAR
```

My strategy is:

```bash
do_while_begin:   # label for the start of do-while
code_block.codes  # execute code block
exp.codes         # check for loop condition
bnez $t0, do_while_begin  # if it is true, jump to the beginning
```

#### `while`

For a while statement with the following structure:

```yacc
WHILE LPAR exp RPAR code_block
```

My strategy is:

```bash
while_begin:   # label for the start of while
exp.codes         # check for loop condition
beqz $t0, while_end  # if it is false, jump to the end
code_block.codes  # execute code block
j while_begin: # start another loop
while_end:     # label for the end of while
```
