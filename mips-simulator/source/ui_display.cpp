#include "simulator.h"
#include <sstream>
#include <iomanip>
#include <bitset>

using namespace std;


void Simulator::display(FILE *fp) {
    fprintf(fp, ".text\n");
    display_code(fp);

    fprintf(fp, "\n");
    fprintf(fp, "registers:\n");
    display_registers(fp);

    fprintf(fp, "\n");
    fprintf(fp, ".data (static, %dB)\n", static_size);
    display_static_data(fp, DISPLAY_STATIC_SIZE);

    fprintf(fp, "\n");
    fprintf(fp, ".data (heap, %dB)\n", heap_size);
    display_heap_data(fp, DISPLAY_HEAP_SIZE);

    fprintf(fp, "\n");
    fprintf(fp, ".data (stack, %dB)\n", MEMORY_SIZE + TEXT_OFFSET - reg[29]);
    display_stack_data(fp, DISPLAY_STACK_SIZE);
}


void Simulator::display_code(FILE *fp) {
    int addr = TEXT_OFFSET;
    int code;
    while (addr < TEXT_OFFSET + code_size) {
        code = mem[addr];
        string addr_str, inst;
        addr_str = int2hex(addr, 8);
        inst = int2binary(code);
        if (addr == get_pc()) {
            fprintf(fp, "--> ");
        } else {
            fprintf(fp, "    ");
        }
        fprintf(fp, "%s   %s, ", addr_str.c_str(), inst.c_str());
        string mips = translate_code_to_asm(code);
        fprintf(fp, "%s\n", mips.c_str());
        addr += 4;
    }
}


void Simulator::display_registers(FILE *fp) {
    for (int i = 0; i < 35; i++) {
        fprintf(fp, "%6s    %2d    %10d    0x%08x\n", 
            reg2name[i].c_str(), i, reg[i], reg[i]);
    }
}


void Simulator::display_data_block(FILE *fp, int start_addr, int n_bytes) {
    int n_rows = n_bytes / 32 + bool(n_bytes % 32);
    for (int i = 0; i < n_rows; i++) {
        int addr = start_addr + i * 4 * 8;
        fprintf(fp, "%s  ", int2hex(addr, 8).c_str());
        // first pass, print as integer
        for (int j = 0; j < DISPLAY_MEM_PER_ROW; j++) {
            fprintf(fp, "%10d", mem[addr + j*4]);
            fprintf(fp, "    ");
        }
        fprintf(fp, "\n            ");
        // second pass, print as hex
        for (int j = 0; j < DISPLAY_MEM_PER_ROW; j++) {
            fprintf(fp, "0x%08x", mem[addr + j*4]);
            fprintf(fp, "    ");
        }
        fprintf(fp, "\n            ");
        // third pass, print as characters
        for (int j = 0; j < DISPLAY_MEM_PER_ROW; j++) {
            char *pt = (char *) &mem[addr + j*4];
            char repr[10] = {};
            int i = 0;
            for (int k = 0; k < 4; k++) {
                if (pt[k] == '\n') {
                    repr[i++] = '\\';
                    repr[i++] = 'n';
                } else if (pt[k] == '\t') {
                    repr[i++] = '\\';
                    repr[i++] = 't';
                } else if (pt[k] == '\r') {
                    repr[i++] = '\\';
                    repr[i++] = 'r';
                } else if (pt[k] == '\0') {
                    repr[i++] = ' ';
                } else {
                    repr[i++] = pt[k];
                }
                repr[i++] = ' ';
            }
            fprintf(fp, "%10s", repr);
            fprintf(fp, "    ");
        }
        fprintf(fp, "\n");
    }
}

void Simulator::display_static_data(FILE *fp, int n_bytes) {
    display_data_block(fp, TEXT_OFFSET+TEXT_SIZE, n_bytes);
}

void Simulator::display_heap_data(FILE *fp, int n_bytes) {
    display_data_block(fp, TEXT_OFFSET+TEXT_SIZE+static_size, n_bytes);
}

void Simulator::display_stack_data(FILE *fp, int n_bytes) {
    display_data_block(fp, TEXT_OFFSET+MEMORY_SIZE-n_bytes, n_bytes);
}


string Simulator::translate_code_to_asm(int code) {
    uint32_t ucode = (uint32_t) code;
    uint32_t opcode = ucode >> 26;
    stringstream mips;
    if (opcode == 0) {
        int rs, rt, rd, sa, funct;
        funct = ucode & 0x3f;
        sa = (ucode >> 6) & 0x1f;
        rd = (ucode >> 11) & 0x1f;
        rt = (ucode >> 16) & 0x1f;
        rs = (ucode >> 21) & 0x1f;
        mips << int2hex(opcode, 2) << ", ";
        mips << funct2name[funct] << ' ' << reg2name[rs] << ' ' 
             << reg2name[rt] << ' ' << reg2name[rd] << ' ' << sa << ' '
             << int2hex(funct, 2);
    } else if (opcode == 0x02 || opcode == 0x03) {
        int opcode = ucode >> 26;
        int imm = ucode & 0x3ffffff;
        mips << int2hex(opcode, 2) << ", ";
        mips << opcode2name[opcode] << ' ' << imm;
    } else {
        int rs, rt, imm, opcode;
        imm = int16_t(ucode & 0xffff);
        rt = (ucode >> 16) & 0x1f;
        rs = (ucode >> 21) & 0x1f;
        opcode = (ucode >> 26);
        mips << int2hex(opcode, 2) << ", ";
        mips << opcode2name[opcode] << ' ' << reg2name[rs] << ' ' 
             << reg2name[rt] << ' ' << imm;
    }
    return mips.str();
}


string Simulator::int2hex(int addr, int n_digits) {
    // sign extension
    uint32_t uaddr = uint32_t(addr);
    stringstream stream;
    stream << "0x" 
           << setfill ('0') << setw(n_digits) 
           << hex << uaddr;
    return stream.str();
}

string Simulator::int2binary(int code) {
    // sign extension
    uint32_t ucode = uint32_t(code);
    bitset<32> bits(ucode);
    return bits.to_string();
}