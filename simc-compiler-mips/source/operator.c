#include "utils.h"
#include <string.h>
#include <stdlib.h>


void logical_or_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    struct code_list *codes;
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, x.mips_code);
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, y.mips_code);
    prepare_exp_inputs(dst, x, y);
    codes = generate_binary_op_codes("or");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void logical_and_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    struct code_list *codes;
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, x.mips_code);
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, y.mips_code);
    prepare_exp_inputs(dst, x, y);
    codes = generate_binary_op_codes("and");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void bitwise_or_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("or");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void bitwise_and_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("and");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void is_equal_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("sub");
    add_mips_code_list_to_list(codes, dst->mips_code);
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, dst->mips_code);
    codes = generate_binary_imm_op_codes("xori", 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void not_equal_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("sub");
    add_mips_code_list_to_list(codes, dst->mips_code);
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void less_than_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("slt");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void less_or_equal_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("sub");
    add_mips_code_list_to_list(codes, dst->mips_code);
    codes = generate_binary_imm_op_codes("slti", 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void greater_than_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    less_or_equal_exp(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_imm_op_codes("xori", 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void greater_or_equal_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    less_than_exp(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_imm_op_codes("xori", 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void shift_left_logical_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("sllv");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void shift_right_arithmetic_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("srav");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void plus_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("add");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void minus_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code_list *codes;
    codes = generate_binary_op_codes("sub");
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void multiply_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code *code;
    /* ignore overflows */
    code = initialize_code();
    strcpy(code->mips.inst, "mult");
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.rt = TEMP_VALUE_REG + 1;
    add_mips_code_to_list(code, dst->mips_code);
    code = initialize_code();
    strcpy(code->mips.inst, "mflo");
    code->mips.rd = TEMP_VALUE_REG;
    add_mips_code_to_list(code, dst->mips_code);
}


void divide_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    initialize_empty_codes(dst);
    prepare_exp_inputs(dst, x, y);
    struct code *code;
    /* ignore remainder */
    code = initialize_code();
    strcpy(code->mips.inst, "div");
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.rt = TEMP_VALUE_REG + 1;
    add_mips_code_to_list(code, dst->mips_code);
    code = initialize_code();
    strcpy(code->mips.inst, "mflo");
    code->mips.rd = TEMP_VALUE_REG;
    add_mips_code_to_list(code, dst->mips_code);
}


void negate_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x) {
    transfer_codes(dst, x);
    struct code *code;
    code = initialize_code();
    strcpy(code->mips.inst, "sub");
    code->mips.rd = TEMP_VALUE_REG;
    code->mips.rs = 0;
    code->mips.rt = TEMP_VALUE_REG;
    add_mips_code_to_list(code, dst->mips_code);
}


void logical_not_exp(struct temp_value_struct *dst, 
        struct temp_value_struct x) {
    transfer_codes(dst, x);
    struct code_list *codes;
    codes = int_to_bool_codes();
    add_mips_code_list_to_list(codes, dst->mips_code);
    codes = generate_binary_imm_op_codes("xori", 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void register_int_to_exp(struct temp_value_struct *dst, int value) {
    initialize_empty_codes(dst);
    struct code *code;
    code = initialize_code();
    strcpy(code->mips.inst, "li");
    code->mips.rt = TEMP_VALUE_REG;
    code->mips.imm = value;
    add_mips_code_to_list(code, dst->mips_code);
    dst->register_num = 1;
    dst->stack_size = 0;
}


void register_id_to_exp(struct temp_value_struct *dst, char *name) {
    initialize_empty_codes(dst);
    struct code_list *codes;
    struct symbol *sym;
    sym = search_symbol_table(name);
    codes = generate_load_codes(sym->offset, TEMP_VALUE_REG, VAR_BASE_REG, 0);
    add_mips_code_list_to_list(codes, dst->mips_code);
    dst->register_num = 1;
    dst->stack_size = 0;
}


void register_array_element_to_exp(struct temp_value_struct *dst, 
        char *name, struct temp_value_struct offset) {
    transfer_codes(dst, offset);
    struct code_list *codes;
    struct code *code;
    struct symbol *sym;
    sym = search_symbol_table(name);
    /* multiply offset by 4 */
    code = initialize_code();
    strcpy(code->mips.inst, "sll");
    code->mips.rt = TEMP_VALUE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.imm = 2;
    add_mips_code_to_list(code, dst->mips_code);
    code = initialize_code();
    strcpy(code->mips.inst, "add");
    code->mips.rd = TEMP_VALUE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.rt = VAR_BASE_REG;
    add_mips_code_to_list(code, dst->mips_code);
    codes = generate_load_codes(sym->offset, TEMP_VALUE_REG, TEMP_VALUE_REG, 0);
    add_mips_code_list_to_list(codes, dst->mips_code);
    dst->register_num = offset.register_num;
    dst->stack_size = offset.stack_size;
}








/*********************************helper functions**************************************/

struct code_list *generate_binary_op_codes(char *inst) {
    struct code_list *codes;
    struct code *code;
    code = initialize_code();
    strcpy(code->mips.inst, inst);
    code->mips.rd = TEMP_VALUE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.rt = TEMP_VALUE_REG + 1;
    codes = initialize_code_list();
    add_mips_code_to_list(code, codes);
    return codes;
}

struct code_list *generate_binary_imm_op_codes(char *inst, int imm) {
    struct code_list *codes;
    struct code *code;
    code = initialize_code();
    strcpy(code->mips.inst, inst);
    code->mips.rt = TEMP_VALUE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.imm = imm;
    codes = initialize_code_list();
    add_mips_code_to_list(code, codes);
    return codes;
}

struct code_list *int_to_bool_codes() {
    struct code_list *codes, *list;
    list = initialize_code_list();
    codes = generate_binary_imm_op_codes("sltiu", 1);
    add_mips_code_list_to_list(codes, list);
    codes = generate_binary_imm_op_codes("xori", 1);
    add_mips_code_list_to_list(codes, list);
    return list;
}


void prepare_exp_inputs(struct temp_value_struct *dst,
        struct temp_value_struct x, 
        struct temp_value_struct y) {
    struct code_list *codes;
    dst->register_num = MAX(MAX(x.register_num, y.register_num), 2);
    dst->stack_size = MAX(y.stack_size, x.stack_size + 1);
    /* calculate value first */
    dst->mips_code->head = y.mips_code->head;
    dst->mips_code->tail = y.mips_code->tail;
    /* push to stack */
    codes = generate_save_codes(0, TEMP_VALUE_REG, STACK_BASE_REG, 1); /* set to 1 so that it can be updated in recursive expressions */
    add_mips_code_list_to_list(codes, dst->mips_code);
    /* calculate offset */
    update_offset(&x, 4);
    add_mips_code_list_to_list(x.mips_code, dst->mips_code);
    /* lw y to $t1 */
    codes = generate_load_codes(0, TEMP_VALUE_REG+1, STACK_BASE_REG, 1);
    add_mips_code_list_to_list(codes, dst->mips_code);
    free(x.mips_code);
    free(y.mips_code);
}