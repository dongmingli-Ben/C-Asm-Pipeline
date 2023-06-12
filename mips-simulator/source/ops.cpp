#include "ops.h"
#include "simulator.h"
#include <cstdint>

using namespace std;


int add_func(int x, int y, int &addr, Memory& mem) {
    return x + y;
}
int addu_func(int x, int y, int &addr, Memory& mem) {
    return int(uint32_t(x) + uint32_t(y));
}
int and_func(int x, int y, int &addr, Memory& mem) {
    return x & y;
}
int jalr_func(int x, int y, int &addr, Memory& mem) {
    int t = addr;
    addr = y;
    return t; // TODO:???
}
int jr_func(int x, int y, int &addr, Memory& mem) {
    addr = x;
    return 0;
}
int mfhi_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int mflo_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int mthi_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int mtlo_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int nor_func(int x, int y, int &addr, Memory& mem) {
    return (!x) & (!y);
}
int or_func(int x, int y, int &addr, Memory& mem) {
    return x | y;
}
int sll_func(int x, int y, int &addr, Memory& mem) {
    return x << y;
}
int sllv_func(int x, int y, int &addr, Memory& mem) {
    return x << y;
}
int slt_func(int x, int y, int &addr, Memory& mem) {
    return x < y ? 1 : 0;
}
int sltu_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) < uint32_t(y) ? 1 : 0;
}
int sra_func(int x, int y, int &addr, Memory& mem) {
    return x >> y;
}
int srav_func(int x, int y, int &addr, Memory& mem) {
    return sra_func(x, y, addr, mem);
}
int srl_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) >> y;
}
int srlv_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) >> y;
}
int sub_func(int x, int y, int &addr, Memory& mem) {
    return x - y;
}
int subu_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) - uint32_t(y);
}
int xor_func(int x, int y, int &addr, Memory& mem) {
    return x ^ y;
}

// end of r type


int addi_func(int x, int y, int &addr, Memory& mem) {
    return x + y;
}
int addiu_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) + uint32_t(y);
}
int andi_func(int x, int y, int &addr, Memory& mem) {
    return x & y;
}
int blez_func(int x, int y, int &addr, Memory& mem) {
    if (x <= 0) {
        y <<= 2;
        addr += y;
    }
    return 0;
}
int lb_func(int x, int y, int &addr, Memory& mem) {
    return (int) mem.at8(x + y);
}
int lbu_func(int x, int y, int &addr, Memory& mem) {
    return (uint32_t) mem.at8(x + y);
}
int lh_func(int x, int y, int &addr, Memory& mem) {
    return (int) mem.at16(x + y);
}
int lhu_func(int x, int y, int &addr, Memory& mem) {
    return (uint32_t) mem.at16(x + y);
}
int lui_func(int x, int y, int &addr, Memory& mem) {
    return x << 16;
}
int lw_func(int x, int y, int &addr, Memory& mem) {
    int tgt = x + y;
    return mem.at32(tgt);
}
int ori_func(int x, int y, int &addr, Memory& mem) {
    return x | y;
}
int sb_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int slti_func(int x, int y, int &addr, Memory& mem) {
    return x < y ? 1 : 0;
}
int sltiu_func(int x, int y, int &addr, Memory& mem) {
    return uint32_t(x) < uint32_t(y) ? 1 : 0;
}
int sh_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int sw_func(int x, int y, int &addr, Memory& mem) {
    return x;
}
int xori_func(int x, int y, int &addr, Memory& mem) {
    return xor_func(x, y, addr, mem);
}
int j_func(int x, int y, int &addr, Memory& mem) {
    addr = (addr & 0xfc000000) | ((x << 2) & 0x03ffffff);
    // return value does not matter
    return 0;
}
int jal_func(int x, int y, int &addr, Memory& mem) {
    int tgt = addr;  // todo: +8???
    addr = (tgt & 0xfc000000) | ((x << 2) & 0x03ffffff);
    return tgt;
}





void register_r_type_functions(map<int, int (*)(int, int, int&, Memory&)> &func_map) {
    func_map.insert({32, add_func});
    func_map.insert({33, addu_func});
    func_map.insert({36, and_func});
    func_map.insert({9, jalr_func});
    func_map.insert({8, jr_func});
    func_map.insert({16, mfhi_func});
    func_map.insert({18, mflo_func});
    func_map.insert({17, mthi_func});
    func_map.insert({19, mtlo_func});
    func_map.insert({39, nor_func});
    func_map.insert({37, or_func});
    func_map.insert({0, sll_func});
    func_map.insert({4, sllv_func});
    func_map.insert({42, slt_func});
    func_map.insert({43, sltu_func});
    func_map.insert({3, sra_func});
    func_map.insert({7, srav_func});
    func_map.insert({2, srl_func});
    func_map.insert({6, srlv_func});
    func_map.insert({34, sub_func});
    func_map.insert({35, subu_func});
    func_map.insert({38, xor_func});
}


void register_other_functions(map<int, int (*)(int, int, int&, Memory&)> &func_map) {
    func_map.insert({8, addi_func});
    func_map.insert({9, addiu_func});
    func_map.insert({12, andi_func});
    func_map.insert({6, blez_func});
    func_map.insert({32, lb_func});
    func_map.insert({36, lbu_func});
    func_map.insert({33, lh_func});
    func_map.insert({37, lhu_func});
    func_map.insert({15, lui_func});
    func_map.insert({35, lw_func});
    func_map.insert({13, ori_func});
    func_map.insert({40, sb_func});
    func_map.insert({10, slti_func});
    func_map.insert({11, sltiu_func});
    func_map.insert({41, sh_func});
    func_map.insert({43, sw_func});
    func_map.insert({14, xori_func});
    func_map.insert({2, j_func});
    func_map.insert({3, jal_func});
}


void register_register_names(map<int, string> &reg_map) {
    reg_map.insert({0, "$zero"});
    reg_map.insert({1, "$at"});
    reg_map.insert({2, "$v0"});
    reg_map.insert({3, "$v1"});
    reg_map.insert({4, "$a0"});
    reg_map.insert({5, "$a1"});
    reg_map.insert({6, "$a2"});
    reg_map.insert({7, "$a3"});
    reg_map.insert({8, "$t0"});
    reg_map.insert({9, "$t1"});
    reg_map.insert({10, "$t2"});
    reg_map.insert({11, "$t3"});
    reg_map.insert({12, "$t4"});
    reg_map.insert({13, "$t5"});
    reg_map.insert({14, "$t6"});
    reg_map.insert({15, "$t7"});
    reg_map.insert({16, "$s0"});
    reg_map.insert({17, "$s1"});
    reg_map.insert({18, "$s2"});
    reg_map.insert({19, "$s3"});
    reg_map.insert({20, "$s4"});
    reg_map.insert({21, "$s5"});
    reg_map.insert({22, "$s6"});
    reg_map.insert({23, "$s7"});
    reg_map.insert({24, "$t8"});
    reg_map.insert({25, "$t9"});
    reg_map.insert({26, "$k0"});
    reg_map.insert({27, "$k1"});
    reg_map.insert({28, "$gp"});
    reg_map.insert({29, "$sp"});
    reg_map.insert({30, "$fp"});
    reg_map.insert({31, "$ra"});
    reg_map.insert({32, "$pc"});
    reg_map.insert({33, "$HI"});
    reg_map.insert({34, "$LO"});
}


void register_opcode2name(map<int, string>& opcode2name) {
    opcode2name.insert({8, "addi"});
    opcode2name.insert({9, "addiu"});
    opcode2name.insert({12, "andi"});
    opcode2name.insert({4, "beq"});
    opcode2name.insert({1, "bgez"});
    opcode2name.insert({7, "bgtz"});
    opcode2name.insert({6, "blez"});
    opcode2name.insert({1, "bltz"});
    opcode2name.insert({5, "bne"});
    opcode2name.insert({32, "lb"});
    opcode2name.insert({36, "lbu"});
    opcode2name.insert({33, "lh"});
    opcode2name.insert({37, "lhu"});
    opcode2name.insert({15, "lui"});
    opcode2name.insert({35, "lw"});
    opcode2name.insert({13, "ori"});
    opcode2name.insert({40, "sb"});
    opcode2name.insert({10, "slti"});
    opcode2name.insert({11, "sltiu"});
    opcode2name.insert({41, "sh"});
    opcode2name.insert({43, "sw"});
    opcode2name.insert({14, "xori"});
    opcode2name.insert({34, "lwl"});
    opcode2name.insert({38, "lwr"});
    opcode2name.insert({42, "swl"});
    opcode2name.insert({46, "swr"});
    opcode2name.insert({2, "j"});
    opcode2name.insert({3, "jal"});
}


void register_funct2name(map<int, string>& funct2name) {
    funct2name.insert({32, "add"});
    funct2name.insert({33, "addu"});
    funct2name.insert({36, "and"});
    funct2name.insert({26, "div"});
    funct2name.insert({27, "divu"});
    funct2name.insert({9, "jalr"});
    funct2name.insert({8, "jr"});
    funct2name.insert({16, "mfhi"});
    funct2name.insert({18, "mflo"});
    funct2name.insert({17, "mthi"});
    funct2name.insert({19, "mtlo"});
    funct2name.insert({24, "mult"});
    funct2name.insert({25, "multu"});
    funct2name.insert({39, "nor"});
    funct2name.insert({37, "or"});
    funct2name.insert({0, "sll"});
    funct2name.insert({4, "sllv"});
    funct2name.insert({42, "slt"});
    funct2name.insert({43, "sltu"});
    funct2name.insert({3, "sra"});
    funct2name.insert({7, "srav"});
    funct2name.insert({2, "srl"});
    funct2name.insert({6, "srlv"});
    funct2name.insert({34, "sub"});
    funct2name.insert({35, "subu"});
    funct2name.insert({12, "syscall"});
    funct2name.insert({38, "xor"});
}