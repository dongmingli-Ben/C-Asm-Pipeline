#include "simulator.h"
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>

using namespace std;

struct r_inst_struct {
    int rs, rt, rd, funct, sa;
};

struct i_inst_struct {
    int rs, rt, imm, opcode;
};


r_inst_struct parse_r_inst(uint32_t code) {
    r_inst_struct inst;
    inst.rs = (code >> 21) & 0x1f;
    inst.rt = (code >> 16) & 0x1f;
    inst.rd = (code >> 11) & 0x1f;
    inst.sa = (code >> 6) & 0x1f;
    inst.funct = code & 0x3f;
    return inst;
}

i_inst_struct parse_i_inst(uint32_t code) {
    i_inst_struct inst;
    inst.rs = (code >> 21) & 0x1f;
    inst.rt = (code >> 16) & 0x1f;
    inst.imm = int16_t(code & 0xffff);
    inst.opcode = code >> 26;
    return inst;
}


void Simulator::execute_div(int code) {
    r_inst_struct inst = parse_r_inst(code);
    int q, r;
    q = inst.rs / inst.rt;
    r = inst.rs % inst.rt;
    reg[HIGH_REG_IDX] = r;
    reg[LOW_REG_IDX] = q;
}

void Simulator::execute_divu(int code) {
    r_inst_struct inst = parse_r_inst(code);
    int q, r;
    q = uint32_t(inst.rs) / uint32_t(inst.rt);
    r = uint32_t(inst.rs) % uint32_t(inst.rt);
    reg[HIGH_REG_IDX] = r;
    reg[LOW_REG_IDX] = q;
}

void Simulator::execute_mult(int code) {
    r_inst_struct inst = parse_r_inst(code);
    int64_t prod;
    prod = int64_t(inst.rs) * int64_t(inst.rt);
    reg[HIGH_REG_IDX] = int32_t(prod >> 32);
    reg[LOW_REG_IDX] = int32_t(prod & 0xffffffff);
}

void Simulator::execute_multu(int code) {
    r_inst_struct inst = parse_r_inst(code);
    uint64_t prod;
    prod = uint64_t(inst.rs) * uint64_t(inst.rt);
    reg[HIGH_REG_IDX] = uint32_t(prod >> 32);
    reg[LOW_REG_IDX] = uint32_t(prod & 0xffffffff);
}

void Simulator::execute_bltz(int code) {
    i_inst_struct inst = parse_i_inst(code);
    assert(inst.rt == 0);
    if (reg[inst.rs] < 0) {
        // branch
        int tgt = get_pc() + inst.imm*4;
        get_pc() = tgt;
    }
}

void Simulator::execute_bgez(int code) {
    i_inst_struct inst = parse_i_inst(code);
    assert(inst.rt == 1);
    if (reg[inst.rs] >= 0) {
        // branch
        int tgt = get_pc() + inst.imm*4;
        get_pc() = tgt;
    }
}

void Simulator::execute_beq(int code) {
    i_inst_struct inst = parse_i_inst(code);
    if (reg[inst.rs] == reg[inst.rt]) {
        // branch
        int tgt = get_pc() + inst.imm*4;
        get_pc() = tgt;
    }
}

void Simulator::execute_bne(int code) {
    i_inst_struct inst = parse_i_inst(code);
    if (reg[inst.rs] != reg[inst.rt]) {
        // branch
        int tgt = get_pc() + inst.imm*4;
        get_pc() = tgt;
    }
}


void Simulator::execute_lwl(int code) {
    i_inst_struct inst = parse_i_inst(code);
    // todo: load unaligned word as BIG ENDIAN???
    // according to https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00086-2B-MIPS32BIS-AFP-6.06.pdf
    // page 250
    int base, tgt;
    base = reg[inst.rs];
    tgt = base + inst.imm;
    int word = (tgt - TEXT_OFFSET) / 4;
    int byte_idx = tgt % 4;
    int shift = byte_idx * 8;
    int value = (mem[word] << shift) | (reg[inst.rt] % (1 << shift));
    reg[inst.rt] = value;
}

void Simulator::execute_lwr(int code) {
    i_inst_struct inst = parse_i_inst(code);
    // todo: load unaligned word as BIG ENDIAN???
    int base, tgt;
    base = reg[inst.rs];
    tgt = base + inst.imm;
    int word = (tgt - TEXT_OFFSET) / 4;
    int byte_idx = tgt % 4;
    int shift = (3 - byte_idx) * 8;
    int value = (mem[word] >> shift) | ((reg[inst.rt] >> shift) << shift);
    reg[inst.rt] = value;
}

void Simulator::execute_swl(int code) {
    i_inst_struct inst = parse_i_inst(code);
    // todo: load unaligned word as BIG ENDIAN???
    int base, tgt;
    base = reg[inst.rs];
    tgt = base + inst.imm;
    int word = (tgt - TEXT_OFFSET) / 4;
    int byte_idx = tgt % 4;
    int shift = byte_idx * 8;
    int value = ((mem[word] >> shift) << shift) | (reg[inst.rt] >> shift);
    reg[inst.rt] = value;
}

void Simulator::execute_swr(int code) {
    i_inst_struct inst = parse_i_inst(code);
    // todo: load unaligned word as BIG ENDIAN???
    int base, tgt;
    base = reg[inst.rs];
    tgt = base + inst.imm;
    int word = (tgt - TEXT_OFFSET) / 4;
    int byte_idx = tgt % 4;
    int shift = (3 - byte_idx) * 8;
    int value = ((mem[word] << shift) >> shift) | (reg[inst.rt] << shift);
    reg[inst.rt] = value;
}


void Simulator::execute_syscall() {
    int code = reg[V0_IDX];
    switch (code)
    {
        case 1:
        {
            fprintf(f_out, "%d", reg[A0_IDX]);
            break;
        }
        case 4:
        {
            fprintf(f_out, "%s", (char *) &mem[reg[A0_IDX]]);
            break;
        }
        case 5:
        {
            fscanf(f_in, "%d\n", &reg[V0_IDX]);
            break;
        }
        case 8:
        {
            char *pt = (char *) &mem[reg[A0_IDX]];
            fgets(pt, reg[A1_IDX], f_in);
            int len = strlen(pt);
            // remove trailing \n to simulate keyboard input
            if (pt[len - 1] == '\n') {
                pt[len - 1] = '\0';
            }
            break;
        }
        case 9:
        {
            int addr = TEXT_OFFSET + TEXT_SIZE + static_size + heap_size;
            int n_words = (reg[A0_IDX] / 4) + bool(reg[A0_IDX] % 4);
            heap_size += n_words * 4;
            reg[V0_IDX] = addr;
            break;
        }
        case 10:
        {
            exit_status = 0;
            throw domain_error("");
        }
        case 11:
        {
            fprintf(f_out, "%c", reg[A0_IDX]);
            break;
        }
        case 12:
        {
            char c = fgetc(f_in);
            reg[V0_IDX] = c;
            // remove following \n (to simulate user input)
            c = fgetc(f_in);
            if (c != '\n') {
                ungetc(c, f_in);
            }
            break;
        }
        case 13:
        {
            // open
            int f = open((char *) &mem[reg[A0_IDX]], reg[A1_IDX], reg[A2_IDX]);
            reg[A0_IDX] = f;
            break;
        }
        case 14:
        {
            // read
            int cnt = read(reg[A0_IDX], (void *) &mem.at8(reg[A1_IDX]), reg[A2_IDX]);
            reg[A0_IDX] = cnt;
            break;
        }
        case 15:
        {
            // write
            int cnt = write(reg[A0_IDX], (void *) &mem.at8(reg[A1_IDX]), reg[A2_IDX]);
            reg[A0_IDX] = cnt;
            break;
        }
        case 16:
        {
            // close
            close(reg[A0_IDX]);
            break;
        }
        case 17:
        {
            // exit2
            exit_status = reg[A0_IDX];
            throw domain_error("");
        }
        default:
        {
            fprintf(f_err, "unknown $v0 value %d in syscall\n", reg[V0_IDX]);
            throw runtime_error("syscall unknown $v0 value");
        }
    }
}