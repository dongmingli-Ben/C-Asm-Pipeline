#ifndef SIM_C_COMPILER_UTILS
#define SIM_C_COMPILER_UTILS

#include <stdio.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define MAX_INST_LEN 10
#define MAX_LABEL_LEN 20
#define VAR_BASE_REG 16
#define STACK_BASE_REG 17
#define TEMP_VALUE_REG 8
#define SYSCALL_REG 2

/*
 * The rd, rs, rt, sa fields do not exactly match the MIPS instructions.
 * For R-type: assume inst rd, rs, rt
 * For I-type: assume inst rt, rs, imm, or inst rt, imm(rs), or inst rs, label
 * For J-type: assume inst label
 * 
 * For example, sll $t0, $t0, 2 is considered to be an I-type instruction although
 * it is actually a R-type instruction
 */
struct mips_struct {
    char inst[MAX_INST_LEN];
    int rd, rs, rt, sa, imm;
    char label[MAX_LABEL_LEN];
    int relative;  // whether the imm is a relative number that needs to be updated by base value
};

struct code {
    char code[20];
    struct mips_struct mips;
    struct code *next;
};

struct code_list {
    struct code *head, *tail;
};

/*
A temporay value is a complete section.
One can obtain the temporary value with
global variables only.
*/
struct temp_value_struct {
    int register_num;  /* number of registers used in calculation */
    // int constant;  /* if it is an integer literal, constant=value, otherwise
    //                   constant=-1 */
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

/*****************************VARIABLE DECLARATIONS*******************************************/

/* Register the src codes into global program codes for later codegen */
void register_instructions_to_codes(struct temp_value_struct src);

/* generate codes that initialize the variables in the declarations */
void initialize_symbols(struct temp_value_struct *dst, 
    struct declaration_struct declarations);

/* register variables in declarations in global symbol tables for later codegen */
void register_symbol_table(struct declaration_struct declarations);

/* make the global symbol table empty */
void initialize_empty_symbol_table();

/*
 * Append symbols in tgt to src and place them to dst. After this operation,
 * both src and tgt will be destroyed and should not be used anymore.
 */
void append_symbols(struct declaration_struct *dst,
    struct declaration_struct src, struct declaration_struct tgt);

/* transfer symbols from src to dst. src should not be used anymore after this operation. */
void transfer_symbols(struct declaration_struct *dst, 
    struct declaration_struct src);

/* register (to dst) a variable with initial value */
void initialized_declaration(struct declaration_struct *dst,
    char *name, int value);

/* register (to dst) an array */
void array_declaration(struct declaration_struct *dst,
    char *name, int length);

/* register (to dst) a variable without initial value */
void uninitialized_declaration(struct declaration_struct *dst, char *name);

/*********************************helper functions**************************************/

struct declaration_struct *initialize_empty_declaration();

/*****************************END OF VARIABLE DECLARATIONS************************************/
/**************************************STATEMENTS***************************************/

/*
 * Append codes in tgt to src and place them to dst. After this operation,
 * both src and tgt will be destroyed and should not be used anymore.
 */
void append_codes(struct temp_value_struct *dst, 
    struct temp_value_struct src, struct temp_value_struct tgt);

/* transfer codes from src to dst. src should not be used anymore after this operation. */
void transfer_codes(struct temp_value_struct *dst, 
    struct temp_value_struct src);

void initialize_empty_codes(struct temp_value_struct *dst);

/* name[offset] = value */
void array_assignment(struct temp_value_struct *dst, char *name, 
    struct temp_value_struct offset, struct temp_value_struct value);

/* name = value */
void id_assignment(struct temp_value_struct *dst, char *name,
    struct temp_value_struct value);

void if_else_statement(struct temp_value_struct *dst, 
    struct temp_value_struct if_statement, struct temp_value_struct else_statement);

void if_statement(struct temp_value_struct *dst,
    struct temp_value_struct condition, struct temp_value_struct statements);

void while_statement(struct temp_value_struct *dst,
    struct temp_value_struct condition, struct temp_value_struct statements);

void do_while_statement(struct temp_value_struct *dst,
    struct temp_value_struct condition, struct temp_value_struct statements);

/* exit the program */
void return_statement(struct temp_value_struct *dst);

void read_statement(struct temp_value_struct *dst, char *name);

void write_statement(struct temp_value_struct *dst, struct temp_value_struct value);

/*********************************helper functions**************************************/

struct temp_value_struct *initialize_empty_temp_value();

/* insert c before tgt in list */
void insert_before(struct code *c, struct code *tgt, struct code_list *list);

/* automatically increment branch label count by 1 */
void get_branch_label(char *label);


/**************************************END OF STATEMENTS***************************************/
/**************************************OPERATORS***************************************/

void logical_or_exp(struct temp_value_struct *dst, 
    struct temp_value_struct x, struct temp_value_struct y);

void logical_and_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void bitwise_or_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void bitwise_and_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void is_equal_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void not_equal_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void less_than_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void less_or_equal_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void greater_than_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void greater_or_equal_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void shift_left_logical_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void shift_right_arithmetic_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void plus_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void minus_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void multiply_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void divide_exp(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

void negate_exp(struct temp_value_struct *dst,
    struct temp_value_struct x);

void logical_not_exp(struct temp_value_struct *dst,
    struct temp_value_struct x);

/* for integer literals */
void register_int_to_exp(struct temp_value_struct *dst, int value);
/* for variables */
void register_id_to_exp(struct temp_value_struct *dst, char *name);
/* for arr[offset] */
void register_array_element_to_exp(struct temp_value_struct *dst,
    char *name, struct temp_value_struct offset);

/*********************************helper functions**************************************/

/*
 * Organize the codes for x and y, and transfer them to dst.
 * Codes of x and y should not be used after this operation.
 * After this operation, x's value should be in $t0, and y's 
 * value should be in $t1.
 * 
 * Note: dst should be initialized before.
 */
void prepare_exp_inputs(struct temp_value_struct *dst,
    struct temp_value_struct x, struct temp_value_struct y);

/* inst $t0, $t0, $t1 */
struct code_list *generate_binary_op_codes(char *inst);

/* inst $t0, $t0, imm */
struct code_list *generate_binary_imm_op_codes(char *inst, int imm);

/* $t0 = bool($t0) */
struct code_list *int_to_bool_codes();

/**************************************END OF OPERATORS***************************************/

void update_offset(struct temp_value_struct *expr, int offset_inc);
struct code *initialize_code();
struct code_list *initialize_code_list();
struct code_list *generate_write_codes(int reg);
/* stack = 1 ==> offset is relative to external stack offset */
struct code_list *generate_save_codes(int offset, int src_register, 
                                      int base_register, int stack);
struct code_list *generate_load_codes(int offset, int tgt_register, 
                                      int base_register, int stack);
struct code_list *generate_read_codes();

void add_mips_code_list_to_list(struct code_list *codes, struct code_list *list);
void add_mips_code_to_list(struct code *code, struct code_list *list);

struct symbol *search_symbol_table(char *name);

void generate_mips_to_file(FILE *out);
void generate_mips_from_struct(struct code_list *codes);
void generate_mips_instruction_from_struct(struct code *code);

/* for debug */
void print_code_list(struct code_list *codes);
void print_code(struct code *code);
void detect_loop(struct code_list *codes);

/*
1. remove repetitive sw, lw
2. integrate li and add/sub to addi
3. integrate move and add/sub
*/
void optimize_mips(struct code_list *codes);
void remove_rep_lw(struct code_list *codes);
void replace_add_sub(struct code_list *codes);
void reduce_move(struct code_list *codes);
void mark_line_for_delete(int line_num, struct code_list *codes);
void delete_lines(struct code_list *codes);

#endif