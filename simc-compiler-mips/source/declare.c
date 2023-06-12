#include "utils.h"
#include <string.h>
#include <stdlib.h>

void initialize_symbols(struct temp_value_struct *dst, 
        struct declaration_struct declarations) {
    /* variables can be searched in global symbol table */
    struct symbol *sym = declarations.head;
    dst->mips_code = initialize_code_list();
    while (sym != NULL) {
        if (sym->initialized) {
            int offset = sym->offset + sym->length - 1;
            struct code *code = initialize_code();
            /* li $t0, value */
            strcpy(code->mips.inst, "li");
            code->mips.rt = TEMP_VALUE_REG;
            code->mips.imm = sym->initial_value;
            add_mips_code_to_list(code, dst->mips_code);
            /* sw */
            struct code_list *codes;
            codes = generate_save_codes(offset, TEMP_VALUE_REG, VAR_BASE_REG, 0);
            add_mips_code_list_to_list(codes, dst->mips_code);
        }
        sym = sym->next;
    }
    dst->register_num = 1;
    dst->stack_size = 0;
}


void append_symbols(struct declaration_struct *dst,
        struct declaration_struct src, struct declaration_struct tgt) {
    // initialize_empty_declaration(dst);
    if (src.head == NULL) {
        dst->head = tgt.head;
        dst->tail = tgt.tail;
    } else {
        dst->head = src.head;
        src.tail->next = tgt.head;
        if (tgt.head == NULL) {
            dst->tail = src.tail;
        } else {
            dst->tail = tgt.tail;
        }
    }
    // free(&src);
    // free(&tgt);
}


void transfer_symbols(struct declaration_struct *dst, 
        struct declaration_struct src) {
    // initialize_empty_declaration(dst);
    dst->head = src.head;
    dst->tail = src.tail;
    // free(&src);
}


void initialized_declaration(struct declaration_struct *dst,
        char *name, int value) {
    struct symbol *symbol;
    symbol = malloc(sizeof(struct symbol));
    strcpy(symbol->name, name);
    symbol->initial_value = value;
    symbol->initialized = 1;
    symbol->length = 1;
    // dst = malloc(sizeof(struct declaration_struct));
    dst->head = symbol;
    dst->tail = symbol;
}


void array_declaration(struct declaration_struct *dst,
        char *name, int length) {
    struct symbol *symbol;
    symbol = malloc(sizeof(struct symbol));
    strcpy(symbol->name, name);
    symbol->initialized = 0;
    symbol->length = length;
    // dst = malloc(sizeof(struct declaration_struct));
    dst->head = symbol;
    dst->tail = symbol;
}


void uninitialized_declaration(struct declaration_struct *dst, char *name) {
    struct symbol *symbol;
    symbol = malloc(sizeof(struct symbol));
    strcpy(symbol->name, name);
    symbol->initialized = 0;
    symbol->length = 1;
    // dst = malloc(sizeof(struct declaration_struct));
    dst->head = symbol;
    dst->tail = symbol;
}


/*********************************helper functions*********************************/

struct declaration_struct *initialize_empty_declaration() {
    struct declaration_struct *dst;
    dst = malloc(sizeof(struct declaration_struct));
    dst->head = NULL;
    dst->tail = NULL;
    return dst;
}