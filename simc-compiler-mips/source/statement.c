#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>


void append_codes(struct temp_value_struct *dst,
        struct temp_value_struct src, struct temp_value_struct tgt) {
    // dst = initialize_empty_temp_value();
    initialize_empty_codes(dst);
    dst->register_num = MAX(src.register_num, tgt.register_num);
    dst->stack_size = MAX(src.stack_size, tgt.stack_size);
    if (src.mips_code->head == NULL) {
        dst->mips_code->head = tgt.mips_code->head;
        dst->mips_code->tail = tgt.mips_code->tail;
    } else {
        dst->mips_code->head = src.mips_code->head;
        src.mips_code->tail->next = tgt.mips_code->head;
        if (tgt.mips_code->head == NULL) {
            dst->mips_code->tail = src.mips_code->tail;
        } else {
            dst->mips_code->tail = tgt.mips_code->tail;
        }
    }
    free(src.mips_code);
    free(tgt.mips_code);
    // free(&src);
    // free(&tgt);
    detect_loop(dst->mips_code);
    assert((dst->mips_code->tail == NULL) || (dst->mips_code->tail->next == NULL));
}


void transfer_codes(struct temp_value_struct *dst, 
        struct temp_value_struct src) {
    // dst = initialize_empty_temp_value();
    initialize_empty_codes(dst);
    dst->register_num = src.register_num;
    dst->stack_size = src.stack_size;
    dst->mips_code->head = src.mips_code->head;
    dst->mips_code->tail = src.mips_code->tail;
    free(src.mips_code);
    // free(&src);
    detect_loop(dst->mips_code);
    assert((dst->mips_code->tail == NULL) || (dst->mips_code->tail->next == NULL));
}


void initialize_empty_codes(struct temp_value_struct *dst) {
    dst->register_num = 0;
    dst->stack_size = 0;
    dst->mips_code = initialize_code_list();
}


void array_assignment(struct temp_value_struct *dst, char *name, 
        struct temp_value_struct offset, struct temp_value_struct value) {
    struct symbol *symbol;
    struct code_list *codes;
    struct code *code;
    symbol = search_symbol_table(name);
    initialize_empty_codes(dst);
    /* $t0 = offset, $t1 = value */
    prepare_exp_inputs(dst, offset, value);
    /* multiply offset by 4 */
    code = initialize_code();
    strcpy(code->mips.inst, "sll");
    code->mips.rt = TEMP_VALUE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    code->mips.imm = 2;
    add_mips_code_to_list(code, dst->mips_code);
    /* add $t0, $t0, var offset */
    code = initialize_code();
    strcpy(code->mips.inst, "add");
    code->mips.rd = TEMP_VALUE_REG;
    code->mips.rt = VAR_BASE_REG;
    code->mips.rs = TEMP_VALUE_REG;
    add_mips_code_to_list(code, dst->mips_code);
    /* sw, base address at $t0 */
    codes = generate_save_codes(symbol->offset, TEMP_VALUE_REG+1, TEMP_VALUE_REG, 0);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void id_assignment(struct temp_value_struct *dst, char *name,
        struct temp_value_struct value) {
    struct symbol *sym;
    sym = search_symbol_table(name);
    transfer_codes(dst, value);
    /* sw */
    struct code_list *codes;
    codes = generate_save_codes(sym->offset, TEMP_VALUE_REG, VAR_BASE_REG, 0);
    add_mips_code_list_to_list(codes, dst->mips_code);
}


void if_else_statement(struct temp_value_struct *dst, 
        struct temp_value_struct if_statement, 
        struct temp_value_struct else_statement) {
    // dst->register_num = MAX(if_statement.register_num, else_statement.register_num);
    // dst->stack_size = MAX(if_statement.stack_size, else_statement.stack_size);
    struct code *code;
    /* add jump right at the end of if-true branch */
    code = initialize_code();
    char label[MAX_LABEL_LEN] = {};
    get_branch_label(label);
    strcpy(code->mips.inst, "j");
    strcpy(code->mips.label, label);
    insert_before(code, if_statement.mips_code->tail, if_statement.mips_code); // todo: check the last instruction is label
    /* add ending label for else statement */
    code = initialize_code();
    strcpy(code->mips.label, label);
    add_mips_code_to_list(code, else_statement.mips_code);
    append_codes(dst, if_statement, else_statement);
    dst->register_num = MAX(if_statement.register_num, else_statement.register_num);
    dst->stack_size = MAX(if_statement.stack_size, else_statement.stack_size);
}


void if_statement(struct temp_value_struct *dst,
        struct temp_value_struct condition, 
        struct temp_value_struct statements) {
    struct code *code;
    /* beqz => jump to the end of if-true branch */
    char label[MAX_LABEL_LEN] = {};
    code = initialize_code();
    get_branch_label(label);
    strcpy(code->mips.label, label);
    strcpy(code->mips.inst, "beqz");
    code->mips.rs = TEMP_VALUE_REG;
    add_mips_code_to_list(code, condition.mips_code);
    /* add label to the end of if-true branch */
    code = initialize_code();
    strcpy(code->mips.label, label);
    add_mips_code_to_list(code, statements.mips_code);
    append_codes(dst, condition, statements);
    dst->register_num = MAX(condition.register_num, statements.register_num);
    dst->stack_size = MAX(condition.stack_size, statements.stack_size);
}


void while_statement(struct temp_value_struct *dst,
        struct temp_value_struct condition, 
        struct temp_value_struct statements) {
    struct code *code;
    code = initialize_code();
    char label_begin[MAX_LABEL_LEN];
    char label_end[MAX_LABEL_LEN];
    get_branch_label(label_begin);
    get_branch_label(label_end);
    strcpy(code->mips.label, label_begin);
    insert_before(code, condition.mips_code->head, condition.mips_code);
    /* beqz => jump to the end of while */
    code = initialize_code();
    strcpy(code->mips.inst, "beqz");
    strcpy(code->mips.label, label_end);
    code->mips.rs = TEMP_VALUE_REG;
    add_mips_code_to_list(code, condition.mips_code);
    /* jump back to condition at the end of loop */
    code = initialize_code();
    strcpy(code->mips.inst, "j");
    strcpy(code->mips.label, label_begin);
    add_mips_code_to_list(code, statements.mips_code);
    /* end label for the loop */
    code = initialize_code();
    strcpy(code->mips.label, label_end);
    add_mips_code_to_list(code, statements.mips_code);
    append_codes(dst, condition, statements);
    dst->register_num = MAX(condition.register_num, statements.register_num);
    dst->stack_size = MAX(condition.stack_size, statements.stack_size);
}


void do_while_statement(struct temp_value_struct *dst,
        struct temp_value_struct condition, 
        struct temp_value_struct statements) {
    struct code *code;
    code = initialize_code();
    char label[MAX_LABEL_LEN];
    get_branch_label(label);
    strcpy(code->mips.label, label);
    insert_before(code, statements.mips_code->head, statements.mips_code);
    /* bnez => jump to loop beginning */
    code = initialize_code();
    strcpy(code->mips.inst, "bnez");
    strcpy(code->mips.label, label);
    code->mips.rs = TEMP_VALUE_REG;
    add_mips_code_to_list(code, condition.mips_code);
    append_codes(dst, statements, condition);
    dst->register_num = MAX(condition.register_num, statements.register_num);
    dst->stack_size = MAX(condition.stack_size, statements.stack_size);
}


void return_statement(struct temp_value_struct *dst) {
    initialize_empty_codes(dst);
    struct code *code;
    code = initialize_code();
    strcpy(code->mips.inst, "li");
    code->mips.rt = SYSCALL_REG;
    code->mips.imm = 10;
    add_mips_code_to_list(code, dst->mips_code);
    code = initialize_code();
    strcpy(code->mips.inst, "syscall");
    add_mips_code_to_list(code, dst->mips_code);
    dst->register_num = 0;
    dst->stack_size = 0;
}


void read_statement(struct temp_value_struct *dst, char *name) {
    struct code_list *codes;
    struct symbol *sym;
    sym = search_symbol_table(name);
    initialize_empty_codes(dst);
    codes = generate_read_codes();
    add_mips_code_list_to_list(codes, dst->mips_code);
    /* sw */
    codes = generate_save_codes(sym->offset, SYSCALL_REG, VAR_BASE_REG, 0);
    add_mips_code_list_to_list(codes, dst->mips_code);
    dst->register_num = 1;
    dst->stack_size = 0;
}


void write_statement(struct temp_value_struct *dst, struct temp_value_struct value) {
    transfer_codes(dst, value);
    struct code_list *codes;
    codes = generate_write_codes(TEMP_VALUE_REG);
    add_mips_code_list_to_list(codes, dst->mips_code);
}



/*********************************helper functions**************************************/

struct temp_value_struct *initialize_empty_temp_value() {
    struct temp_value_struct *temp;
    temp = malloc(sizeof(struct temp_value_struct));
    temp->mips_code = initialize_code_list();
    temp->register_num = -1;
    // temp->base_stack_offset = 0;
    temp->stack_size = 0;
    return temp;
}

void insert_before(struct code *c, struct code *tgt, struct code_list *list) {
    struct code *code;
    code = list->head;
    if (code == tgt) {
        /* insert in the front */
        list->head = c;
        c->next = tgt;
        return;
    }
    while (code != NULL && code->next != tgt) {
        code = code->next;
    } 
    if (code == NULL) {
        /* tgt not found */
        assert(0);
    }
    code->next = c;
    c->next = tgt;
}

void get_branch_label(char *label) {
    /*
    * Global counters for branches
    */
    static int BRANCH_NUM = 0;
    sprintf(label, "B_%d_end", BRANCH_NUM);
    BRANCH_NUM++;
}