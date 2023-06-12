#include "simulator.h"
#include <stdexcept>


using namespace std;


op_struct Simulator::parse_code(uint32_t code) {
    uint32_t opcode = code >> 26;
    op_struct op;
    if (opcode == 0) {
        op = parse_r_type(code);
    } else if (opcode == 0x02 || opcode == 0x03) {
        op = parse_j_type(code);
    } else {
        op = parse_i_type(code);
    }
    return op;
}


op_struct Simulator::parse_r_type(uint32_t code) {
    op_struct op;
    int rs, rt, rd, sa, funct;
    funct = code & 0x3f;
    sa = (code >> 6) & 0x1f;
    rd = (code >> 11) & 0x1f;
    rt = (code >> 16) & 0x1f;
    rs = (code >> 21) & 0x1f;
    // do not need to consider syscall
    switch (funct)
    {
    case 0x20 ... 0x27: case 0x2a ... 0x2b:
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        op.in2.type = location::REGISTER;
        op.in2.reg = rt;
        op.out.type = location::REGISTER;
        op.out.reg = rd;
        break;
    case 0x04: case 0x06 ... 0x07:
        op.in1.type = location::REGISTER;
        op.in1.reg = rt;
        op.in2.type = location::REGISTER;
        op.in2.reg = rs;
        op.out.type = location::REGISTER;
        op.out.reg = rd;
        break;
    case 0x00: case 0x02 ... 0x03:
        op.in1.type = location::REGISTER;
        op.in1.reg = rt;
        op.in2.type = location::IMM;
        op.in2.imm = sa;
        op.out.type = location::REGISTER;
        op.out.reg = rd;
        break;
    // no need to consider div, divu, mult, multu
    case 0x09: 
        // jalr
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        op.out.type = location::REGISTER;
        op.out.reg = 31;
        break;
    case 0x08:
        // jr
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        break;
    case 0x10:
        op.in1.type = location::REGISTER;
        op.in1.reg = HIGH_REG_IDX;
        op.out.type = location::REGISTER;
        op.out.reg = rd;
        break;
    case 0x12:
        op.in1.type = location::REGISTER;
        op.in1.reg = LOW_REG_IDX;
        op.out.type = location::REGISTER;
        op.out.reg = rd;
        break;
    case 0x11:
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        op.out.type = location::REGISTER;
        op.out.reg = HIGH_REG_IDX;
        break;
    case 0x13:
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        op.out.type = location::REGISTER;
        op.out.reg = LOW_REG_IDX;
        break;
    
    default:
        fprintf(f_err, "unknown function code %d\n", funct);
        throw runtime_error("unknown function code");
        break;
    }
    op.op_func = funct_code2op_func[funct];
    return op;
}


op_struct Simulator::parse_i_type(uint32_t code) {
    int rs, rt, imm, opcode;
    op_struct op;
    imm = code & 0xffff;
    rt = (code >> 16) & 0x1f;
    rs = (code >> 21) & 0x1f;
    opcode = (code >> 26);
    switch (opcode)
    {
    case 0x08 ... 0x0e:
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        op.in2.type = location::IMM;
        op.in2.imm = int16_t(imm);
        op.out.type = location::REGISTER;
        op.out.reg = rt;
        break;
    case 0x20 ... 0x26: 
        op.in1.type = location::IMM;
        op.in1.imm = int16_t(imm);
        op.in2.type = location::REGISTER;
        op.in2.reg = rs;
        op.out.type = location::REGISTER;
        op.out.reg = rt;
        break;
    case 0x28 ... 0x2b: case 0x2e:
        op.in1.type = location::REGISTER;
        op.in1.reg = rt;
        op.out.type = location::MEMORY;
        op.out.base = rs;
        op.out.offset = int16_t(imm);
        switch (opcode)
        {
            case 0x28: op.out.size = 1; break;
            case 0x29: op.out.size = 2; break;
            case 0x2b: op.out.size = 4; break;
        }
        break;
    case 0x06 ... 0x07:
        op.in1.type = location::REGISTER;
        op.in1.reg = rs;
        if (rt != 0) {
            fprintf(f_err, "rt must be 0 for opcode %d\n", opcode);
            throw invalid_argument("rt must be 0 for bgtz and blez");
        }
        op.in2.type = location::IMM;
        op.in2.imm = int16_t(imm);
        break;
    case 0x0f:
        // lui
        op.in1.type = location::IMM;
        op.in1.imm = int16_t(imm);
        op.in2.type = location::REGISTER;
        op.in2.reg = rt;
        op.out.type = location::REGISTER;
        op.out.reg = rt;
        break;
    
    default:
        fprintf(f_err, "unknown opcode %d\n", opcode);
        throw runtime_error("unknown opcode");
    }
    op.op_func = op_code2op_func[opcode];
    return op;
}


op_struct Simulator::parse_j_type(uint32_t code) {
    int opcode = code >> 26;
    op_struct op;
    if (opcode == 0x02) {
        // jr
        op.out.type = location::UNK;
    } else {
        // jal
        op.out.type = location::REGISTER;
        op.out.reg = 31;
    }
    op.in1.type = location::IMM;
    op.in1.imm = code & 0x3ffffff;
    op.op_func = op_code2op_func[opcode];
    return op;
}