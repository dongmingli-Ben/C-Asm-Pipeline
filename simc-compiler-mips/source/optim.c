#include "utils.h"
#include <string.h>
#include <stdlib.h>


#define SPECIAL_REGISTER_NUM 9
const int SPECIAL_REGISTERS[] = {0, 1, 2, 3, 4, 5, 6, 7, 31};

void optimize_mips(struct code_list *codes) {
    /* 1. lw after sw */
    remove_rep_lw(codes);
    /* 2. li with add/sub */
    replace_add_sub(codes);
    /* 3. move with add/sub */
    reduce_move(codes);
}


void remove_rep_lw(struct code_list *codes) {
    if (codes->head == NULL || codes->head == codes->tail) {
        return;
    }
    struct code *c, *prev_c;
    prev_c = codes->head;
    c = prev_c->next;
    while (c != NULL) {
        if (strcmp(prev_c->mips.inst, "sw") == 0 
                && strcmp(c->mips.inst, "lw") == 0
                && prev_c->mips.rt == c->mips.rt
                && prev_c->mips.rs == c->mips.rs
                && prev_c->mips.imm == c->mips.imm) {
            prev_c->next = c->next;
            free(c);
            c = prev_c->next;
            continue;
        }
        prev_c = c;
        c = c->next;
    }
}


int is_special_register(int reg) {
    for (int i = 0; i < SPECIAL_REGISTER_NUM; i++) {
        if (SPECIAL_REGISTERS[i] == reg) {
            return 1;
        }
    }
    return 0;
}

/*
Cases where reducing li is not possible:
1. imm1 +/- imm2
2. imm - reg
2. imm being an address in lw/sw (not possible in micro language)
3. move (not dealing with move right now, can be dealt with by replacing move with add)
*/
void replace_add_sub(struct code_list *codes) {
    int register_imm[32] = {};
    int register_def_line[32] = {}; /* line starts from 1 */
    int register_remain_ref[32] = {}; /* remaining non-replacable reference to the register */
    struct code *code;
    code = codes->head;
    int line = 1;
    while (code != NULL) {
        if (strcmp(code->mips.inst, "li") == 0) {
            if (is_special_register(code->mips.rt)) {
                /* remove repetitive li with same value */
                if (register_imm[code->mips.rt] == code->mips.imm
                        && register_def_line[code->mips.rt] > 0
                        && (code->mips.rt != 2
                            || code->mips.imm != 5)) { /* $v0 = 5 ==> read_int, change the $v0 value */
                    mark_line_for_delete(line, codes);
                } else {
                    register_imm[code->mips.rt] = code->mips.imm;
                    register_def_line[code->mips.rt] = line;
                }
                code = code->next;
                line++;
                continue;
            }
            if (register_remain_ref[code->mips.rt] == 0
                    && register_def_line[code->mips.rt] > 0) {
                /* reduce the li line */
                mark_line_for_delete(register_def_line[code->mips.rt], codes);
            }
            register_def_line[code->mips.rt] = line;
            register_imm[code->mips.rt] = code->mips.imm;
            register_remain_ref[code->mips.rt] = 0;
            code = code->next;
            line++;
            continue;
        }
        /* do not handle where dest is special register */
        if (strcmp(code->mips.inst, "add") == 0 || strcmp(code->mips.inst, "sub") == 0) {
            int imm_cnt = 0;
            if (register_def_line[code->mips.rt]) {
                imm_cnt++;
                code->mips.imm = code->mips.inst[0] == 'a' ? register_imm[code->mips.rt] :
                    -register_imm[code->mips.rt];
            }
            /* check rs, mips do not support 2 imm */
            if (register_def_line[code->mips.rs]) {
                if (imm_cnt == 0 && code->mips.inst[0] == 'a') {
                    /* needs to be add, so that the position can swap */
                    code->mips.imm = register_imm[code->mips.rs];
                    code->mips.rs = code->mips.rt;
                    imm_cnt++;
                } else {
                    /* cannot replace rs */
                    register_remain_ref[code->mips.rs]++;
                }
            }
            /* check dest register */
            if (register_def_line[code->mips.rd]) {
                if (register_remain_ref[code->mips.rd] == 0
                        && register_def_line[code->mips.rd] > 0
                        && is_special_register(code->mips.rd) == 0) {
                    mark_line_for_delete(register_def_line[code->mips.rd], codes);
                }
                register_def_line[code->mips.rd] = 0;
                register_remain_ref[code->mips.rd] = 0;
            }
            /* add/sub ==> addi */
            if (imm_cnt) {
                strcpy(code->mips.inst, "addi");
                code->mips.rt = code->mips.rd;
            }
        }
        /* check lw */
        if (strcmp(code->mips.inst, "lw") == 0) {
            if (register_remain_ref[code->mips.rt] == 0
                    && register_def_line[code->mips.rt] > 0
                    && is_special_register(code->mips.rt) == 0) {
                mark_line_for_delete(register_def_line[code->mips.rt], codes);
            }
            register_remain_ref[code->mips.rt] = 0;
            register_def_line[code->mips.rt] = 0;
        }
        /* check sw */
        if (strcmp(code->mips.inst, "sw") == 0) {
            register_remain_ref[code->mips.rt]++;
        }
        /* check move */
        if (strcmp(code->mips.inst, "move") == 0) {
            /* assume no move $x, $x kind of instruction */
            register_remain_ref[code->mips.rs]++;
            if (register_remain_ref[code->mips.rt] == 0
                    && register_def_line[code->mips.rt] > 0
                    && is_special_register(code->mips.rt) == 0) {
                mark_line_for_delete(register_def_line[code->mips.rt], codes);
            }
            register_remain_ref[code->mips.rt] = 0;
            register_def_line[code->mips.rt] = 0;
        }
        code = code->next;
        line++;
    }
    delete_lines(codes);
}


void delete_lines(struct code_list *codes) {
    struct code *code, *tmp;
    int line = 1;
    while (codes->head->mips.inst[0] == '\0') {
        code = codes->head;
        codes->head = code->next;
        free(code);
        line++;
    }
    code = codes->head;
    while (code->next != NULL) {
        if (code->next->mips.inst[0] == '\0') {
            tmp = code->next;
            code->next = tmp->next;
            free(tmp);
            continue;
        }
        code = code->next;
        line++;
    }
    codes->tail = code;
}


void mark_line_for_delete(int line_num, struct code_list *codes) {
    struct code *code;
    code = codes->head;
    for (int i = 1; i < line_num; i++) {
        code = code->next;
    }
    memset(code->mips.inst, 0, MAX_INST_LEN);
    // printf("delete line %d\n", line_num);
}

/*
move only occur when WRITE. The source register will not be used again.
It is save to combine move with add/sub/addi/lw/li
*/
void reduce_move(struct code_list *codes) {
    struct code *code, *next;
    int line = 1;
    code = codes->head;
    while (code->next != NULL) {
        /* assume no 2 consecutiive move instructions */
        if (strcmp(code->next->mips.inst, "move") == 0) {
            int remove = 0;
            if (strcmp(code->mips.inst, "add") == 0
                    || strcmp(code->mips.inst, "sub") == 0) {
                code->mips.rd = code->next->mips.rt;
                remove = 1;
            } else if (strcmp(code->mips.inst, "addi") == 0) {
                code->mips.rt = code->next->mips.rt;
                remove = 1;
            } else if (strcmp(code->mips.inst, "lw") == 0
                    || strcmp(code->mips.inst, "li") == 0) {
                code->mips.rt = code->next->mips.rt;
                remove = 1;
            }
            if (remove) {
                mark_line_for_delete(line+1, codes);
            }
        }
        line++;
        code = code->next;
    }
    delete_lines(codes);
}