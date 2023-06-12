#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_INST_NUM 2
#define IMM_INST_NUM 1
#define TRI_INST_NUM 7
#define MOV_INST_NUM 1
#define BINARY_INST_NUM 2
#define UNARY_INST_NUM 1
#define OPIMM_INST_NUM 5
#define BRANCH_INST_NUM 2
#define JUMP_INST_NUM 1
const char *MEM_INST[] = {"sw", "lw"};
const char *IMM_INST[] = {"li"};
const char *TRI_INST[] = {"add", "sub", "slt", "or", "and", 
    "srav", "sllv"};
const char *MOV_INST[] = {"move"};
const char *BINARY_INST[] = {"mult", "div"};
const char *UNARY_INST[] = {"mflo"};
const char *OPIMM_INST[] = {"addi", "slti", "sltiu", "xori", "sll"};
const char *BRANCH_INST[] = {"beqz", "bnez"};
const char *JUMP_INST[] = {"j"};


/*
Assumptions!!!
* label data points to the start of variable addresses
* address of the starting variable is stored in $s0 (and should not be changed)
*/
struct code_list PROGRAM_CODE = { .head = NULL, .tail = NULL };
struct symbol *SYMBOL_TABLE_HEAD = NULL, *SYMBOL_TABLE_TAIL = NULL;
int NUM_SYMBOLS = 0;
int NEXT_SYMBOL_OFFSET = 0;
int STACK_SIZE = 0;


void initialize_empty_symbol_table() {
    if (SYMBOL_TABLE_HEAD != NULL) {
        /* destroy previous table */
        struct symbol *sym = SYMBOL_TABLE_HEAD;
        struct symbol *prev;
        while (sym != NULL) {
            prev = sym;
            sym = sym->next;
            free(prev);
        }
        free(sym);
    }
    SYMBOL_TABLE_HEAD = NULL;
    SYMBOL_TABLE_TAIL = NULL;
}

void register_symbol_table(struct declaration_struct declarations) {
    /* also update symbol offsets */
    struct symbol *sym = declarations.head;
    SYMBOL_TABLE_HEAD = declarations.head;
    SYMBOL_TABLE_TAIL = declarations.tail;
    while (sym != NULL) {
        sym->offset = NEXT_SYMBOL_OFFSET;
        NUM_SYMBOLS++;
        NEXT_SYMBOL_OFFSET += sizeof(int) * sym->length;
        sym = sym->next;
    }
}

void register_instructions_to_codes(struct temp_value_struct src) {
    add_mips_code_list_to_list(src.mips_code, &PROGRAM_CODE);
    STACK_SIZE = src.stack_size;
    // todo: free src mips code ???
    free(src.mips_code);
}

/*================================helper functions==============================*/

void update_offset(struct temp_value_struct *expr, int offset_inc) {
    struct code *code;
    code = expr->mips_code->head;
    while (code != NULL) {
        if (code->mips.relative) {
            code->mips.imm += offset_inc;
            STACK_SIZE = STACK_SIZE >= (code->mips.imm/4 + 1) ? STACK_SIZE :
                (code->mips.imm/4 + 1);
        } 
        code = code->next;
        // if (code == expr->mips_code->tail) {
        //     printf("currently at tail\n");
        // }
        // if (code == expr->mips_code->head) {
        //     printf("Loop in code! at head again!\n");
        //     assert(0);
        // }
    }
}


struct code *initialize_code() {
    struct code *code;
    code = malloc(sizeof(struct code));
    code->next = NULL;
    memset(code->mips.inst, 0, MAX_INST_LEN);
    memset(code->mips.label, 0, MAX_LABEL_LEN);
    code->mips.rd = -1;
    code->mips.rs = -1;
    code->mips.rt = -1;
    code->mips.imm = -1;
    code->mips.sa = 0;
    code->mips.relative = 0;
    return code;
}

struct code_list *initialize_code_list() {
    struct code_list *list;
    list = malloc(sizeof(struct code_list));
    list->head = NULL;
    list->tail = NULL;
    return list;
}


struct code_list *generate_write_codes(int reg) {
    int i = 0;
    struct code_list *codes;
    struct code *c;
    char char_code[4];
    codes = initialize_code_list();

    c = initialize_code();
    strcpy(c->mips.inst, "move");
    c->mips.rt = 4;
    c->mips.rs = reg;
    add_mips_code_to_list(c, codes);

    c = initialize_code();
    strcpy(c->mips.inst, "li");
    c->mips.rt = 2;
    c->mips.imm = 1;
    add_mips_code_to_list(c, codes);

    c = initialize_code();
    strcpy(c->mips.inst, "syscall");
    add_mips_code_to_list(c, codes);

    /* add \n to the end */
    c = initialize_code();
    strcpy(c->mips.inst, "li");
    c->mips.rt = 2;
    c->mips.imm = 11;
    add_mips_code_to_list(c, codes);

    c = initialize_code();
    strcpy(c->mips.inst, "li");
    c->mips.rt = 4;
    c->mips.imm = '\n';
    add_mips_code_to_list(c, codes);

    c = initialize_code();
    strcpy(c->mips.inst, "syscall");
    add_mips_code_to_list(c, codes);


    return codes;
}


struct code_list *generate_save_codes(int offset, int src_register, 
                                      int base_register, int stack) {
    struct code_list *codes = initialize_code_list();
    struct code *inst = initialize_code();
    inst->next = NULL;
    strcpy(inst->mips.inst, "sw");
    inst->mips.rt = src_register;
    inst->mips.imm = offset;
    inst->mips.rs = base_register;
    if (stack) {
        inst->mips.relative = 1;
    } 
    codes->head = inst;
    codes->tail = inst;
    return codes;
}


struct code_list *generate_load_codes(int offset, int tgt_register, 
                                      int base_register, int stack) {
    struct code_list *codes = initialize_code_list();
    struct code *inst = initialize_code();
    inst->next = NULL;
    strcpy(inst->mips.inst, "lw");
    inst->mips.rt = tgt_register;
    inst->mips.imm = offset;
    inst->mips.rs = base_register;
    if (stack) {
        inst->mips.relative = 1;
    } 
    codes->head = inst;
    codes->tail = inst;
    return codes;
}


struct code_list *generate_read_codes() {
    struct code_list *codes = initialize_code_list();
    struct code *code = initialize_code();

    strcpy(code->mips.inst, "li");
    code->mips.rt = 2;
    code->mips.imm = 5;
    add_mips_code_to_list(code, codes);

    code = initialize_code();
    strcpy(code->mips.inst, "syscall");
    add_mips_code_to_list(code, codes);

    return codes;
}

void add_mips_code_list_to_list(struct code_list *codes, struct code_list *list) {
    if (list->head == NULL) {
        list->head = codes->head;
        list->tail = codes->tail;
    } else {
        list->tail->next = codes->head;
        list->tail = codes->tail;
    }
    detect_loop(list);
    // print_code_list(list);
}

void add_mips_code_to_list(struct code *code, struct code_list *list) {
    struct code_list t;
    t.head = code;
    t.tail = code;
    add_mips_code_list_to_list(&t, list);
}


struct symbol *search_symbol_table(char *name) {
    struct symbol * ps;
    ps = SYMBOL_TABLE_HEAD;
    while (ps != NULL) {
        if (strcmp(ps->name, name) == 0) {
            return ps;
        }
        ps = ps->next;
    }
    return ps;
}


void generate_mips_to_file(FILE *out) {
    /* data section */
    fputs(".data\n", out);
    char var_num[10];
    sprintf(var_num, "%d", NEXT_SYMBOL_OFFSET+1); /* in case no ID */
    char var[20] = "var: .byte 0 : ";
    strcat(var, var_num);
    strcat(var, "\n");
    fputs(var, out);
    char stack[20] = "stack: .word 0: ";
    char stack_size[10];
    sprintf(stack_size, "%d", STACK_SIZE+1); /* in case it is 0 */
    strcat(stack, stack_size);
    strcat(stack, "\n");
    fputs(stack, out);
    /* text section */
    fputs(".text\n", out);
    /* load base address to $s0 */
    fprintf(out, "    la $%d, var\n", VAR_BASE_REG);
    /* load stack base address to $s1 */
    fprintf(out, "    la $%d, stack\n", STACK_BASE_REG);
    /* optimization */
    // optimize_mips(&PROGRAM_CODE);
    generate_mips_from_struct(&PROGRAM_CODE);
    struct code *code;
    code = PROGRAM_CODE.head;
    while (code != NULL) {
        // fputs("    ", out);
        fputs(code->code, out);
        fputc('\n', out);
        code = code->next;
    }
    /* clean up allocated memory */
    struct code *next;
    code = PROGRAM_CODE.head;
    while (code != NULL) {
        next = code->next;
        free(code);
        code = next;
    }
    /* symbol table */
    struct symbol *sym, *next_sym;
    sym = SYMBOL_TABLE_HEAD;
    while (sym != NULL) {
        next_sym = sym->next;
        free(sym);
        sym = next_sym;
    }
}


void generate_mips_from_struct(struct code_list *codes) {
    struct code *code;
    code = codes->head;
    char imm_str[10];
    while (code != NULL) {
        generate_mips_instruction_from_struct(code);
        code = code->next;
    }
}


void generate_mips_instruction_from_struct(struct code *code) {
    if (strlen(code->mips.inst) == 0) {
        /* this is a label */
        strcpy(code->code, code->mips.label);
        strcat(code->code, ":");
        return;
    }
    strcpy(code->code, "    ");
    strcat(code->code, code->mips.inst);
    strcat(code->code, " ");
    char imm_str[20];
    for (int i = 0; i < TRI_INST_NUM; i++) {
        if (strcmp(code->mips.inst, TRI_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rd);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", $");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", $");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < BINARY_INST_NUM; i++) {
        if (strcmp(code->mips.inst, BINARY_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", $");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < UNARY_INST_NUM; i++) {
        if (strcmp(code->mips.inst, UNARY_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rd);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < IMM_INST_NUM; i++) {
        if (strcmp(code->mips.inst, IMM_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", ");
            sprintf(imm_str, "%d", code->mips.imm);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < MEM_INST_NUM; i++) {
        if (strcmp(code->mips.inst, MEM_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", ");
            sprintf(imm_str, "%d", code->mips.imm);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, "($");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            strcat(code->code, ")");
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < MOV_INST_NUM; i++) {
        if (strcmp(code->mips.inst, MOV_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", $");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < OPIMM_INST_NUM; i++) {
        if (strcmp(code->mips.inst, OPIMM_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rt);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", $");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", ");
            sprintf(imm_str, "%d", code->mips.imm);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);
            return;
        }
    }

    for (int i = 0; i < BRANCH_INST_NUM; i++) {
        if (strcmp(code->mips.inst, BRANCH_INST[i]) == 0) {
            strcat(code->code, "$");
            sprintf(imm_str, "%d", code->mips.rs);
            strcat(code->code, imm_str);
            memset(imm_str, 0, 20);

            strcat(code->code, ", ");
            strcat(code->code, code->mips.label);
            return;
        }
    }

    for (int i = 0; i < JUMP_INST_NUM; i++) {
        if (strcmp(code->mips.inst, JUMP_INST[i]) == 0) {
            strcat(code->code, code->mips.label);
            return;
        }
    }
    /* syscall need no further processing */
}

/* debug helpers */

void print_code_list(struct code_list *codes) {
    struct code *code;
    code = codes->head;
    printf("codes at %lld\n", (long long) codes);
    while (code != NULL) {
        print_code(code);
        code = code->next;
    }
}

void print_code(struct code *code) {
    if (strlen(code->mips.inst) == 0) {
        printf("label: %s\n", code->mips.label);
    } else {
        printf("inst: %s, rd %d, rs %d, rt %d, imm %d\n",
            code->mips.inst, code->mips.rd, code->mips.rs, code->mips.rt,
            code->mips.imm);
    }
}

void detect_loop(struct code_list *codes) {
    /* detect loop in list */
    struct code *a, *b;
    a = codes->head;
    b = codes->head;
    while (a != NULL && a->next != NULL) {
        a = a->next->next;
        b = b->next;
        if (a == b) {
            printf("loop detected in code!\n");
            assert(0);
        }
    }
}