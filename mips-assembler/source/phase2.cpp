#include <iostream>
#include <regex>
#include "LabelTable.h"
#include "utils.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

/*
Convert mips instruments to binary code
*/
uint32_t inst_to_code(line_struct & line, const LabelTable & table,
        const json & inst_map);

enum class INST_TYPE {
    R_TYPE,
    I_TYPE,
    J_TYPE,
    UNK
};

struct inst_struct {
    uint16_t rd, rs, rt, sa, opcode, funct, imm;
    uint32_t target;
    INST_TYPE type;
};

inst_struct parse_instruction(line_struct & line, const LabelTable & table,
        const json & inst_map);

void initialize_inst(inst_struct & inst) {
    inst.rd = 0;
    inst.rs = 0;
    inst.rt = 0;
    inst.sa = 0;
    inst.opcode = 0;
    inst.funct = 0;
    inst.imm = 0;
    inst.target = 0;
    inst.type = INST_TYPE::UNK;
}


unordered_map<string, int> REGISTER_MAP {
    {"$zero", 0},
    {"$at", 1},
    {"$v0", 2},
    {"$v1", 3},
    {"$a0", 4},
    {"$a1", 5},
    {"$a2", 6},
    {"$a3", 7},
    {"$t0", 8},
    {"$t1", 9},
    {"$t2", 10},
    {"$t3", 11},
    {"$t4", 12},
    {"$t5", 13},
    {"$t6", 14},
    {"$t7", 15},
    {"$s0", 16},
    {"$s1", 17},
    {"$s2", 18},
    {"$s3", 19},
    {"$s4", 20},
    {"$s5", 21},
    {"$s6", 22},
    {"$s7", 23},
    {"$t8", 24},
    {"$t9", 25},
    {"$k0", 26},
    {"$k1", 27},
    {"$gp", 28},
    {"$sp", 29},
    {"$fp", 30},
    {"$ra", 31},
};


/*
Write the binary code to output.txt
*/
LabelTable pass2 (char * filename, LabelTable table) {
    ifstream file, inst_file;
    ofstream out_file;
    file.open(filename, ios::in);
    out_file.open("output.txt", ios::out);

    inst_file.open("instructions.json", ios::in);
    json inst_map = json::parse(inst_file);

    line_struct line;
    int addr = 0x400000;
    uint32_t code;
    string b_code;
    while ((line = parse_line(file, addr)).addr != 0) {
        code = inst_to_code(line, table, inst_map);
        b_code = int_to_binary_str(code);
        out_file << b_code << endl;
    }

    file.close();
    out_file.close();
    return table;
}


uint32_t inst_to_code(line_struct & line, const LabelTable & table,
        const json & inst_map) {
    inst_struct inst = parse_instruction(line, table, inst_map);
    uint32_t code = 0;
    
    switch (inst.type)
    {
    case INST_TYPE::R_TYPE:
        code |= inst.funct;
        code |= (inst.sa << 6);
        code |= (inst.rd << 11);
        code |= (inst.rt << 16);
        code |= (inst.rs << 21);
        break;
    case INST_TYPE::I_TYPE:
        code |= inst.imm;
        code |= (inst.rt << 16);
        code |= (inst.rs << 21);
        code |= (inst.opcode << 26);
        break;
    case INST_TYPE::J_TYPE:
        code |= inst.target;
        code |= (inst.opcode << 26);
        break;
    
    default:
        break;
    }
    return code;
}


INST_TYPE parse_instruction_type(string inst, const json & inst_map) {
    int i = inst.find_first_of(' ');
    if (i == inst.npos) {
        cerr << inst << " is not a valid instruction\n";
        exit(EXIT_FAILURE);
    }
    inst = inst.substr(0, i);
    json detail = inst_map[inst];
    string type = detail["type"].get<string>();
    INST_TYPE inst_type;
    if (type == "R"s) inst_type = INST_TYPE::R_TYPE;
    else if (type == "I"s) inst_type = INST_TYPE::I_TYPE;
    else inst_type = INST_TYPE::J_TYPE;
    return inst_type;
}


inst_struct parse_instruction(line_struct & line, const LabelTable & table,
        const json & inst_map) {
    inst_struct inst;
    initialize_inst(inst);
    string inst_str = line.inst;
    int i = inst_str.find_first_of(' ');
    if (i == inst_str.npos) {
        cerr << inst_str << " is not a valid instruction\n";
        exit(EXIT_FAILURE);
    }
    string arg = inst_str.substr(i);
    inst_str = inst_str.substr(0, i);
    json detail = inst_map[inst_str];
    if (detail["type"].get<string>() == "R"s) {
        inst.opcode = 0;
        inst.type = INST_TYPE::R_TYPE;
        inst.funct = detail["funct"].get<uint32_t>();
    } else if (detail["type"].get<string>() == "I"s) {
        inst.opcode = detail["opcode"].get<uint32_t>();
        inst.type = INST_TYPE::I_TYPE;
    } else {
        inst.opcode = detail["opcode"].get<uint32_t>();
        inst.type = INST_TYPE::J_TYPE;
    }

    string pattn = detail["regex"].get<string>();
    vector<string> fields = detail["fields"].get<vector<string>>();

    regex regexp(pattn);
    smatch sm;
    regex_search(arg, sm, regexp);

    for (i = 1; i < sm.size(); i++) {
        if (fields[i-1] == "rt"s) inst.rt = REGISTER_MAP[sm[i]];
        else if (fields[i-1] == "rs"s) inst.rs = REGISTER_MAP[sm[i]];
        else if (fields[i-1] == "rd"s) inst.rd = REGISTER_MAP[sm[i]];
        else if (fields[i-1] == "sa"s) inst.sa = REGISTER_MAP[sm[i]];
        else if (fields[i-1] == "immediate"s) inst.imm = stoi(sm[i]);
        else if (fields[i-1] == "label"s) {
            if (inst.imm != 0) {
                cerr << "imm is specified!" << endl;
                exit(EXIT_FAILURE);
            }
            string str = sm[i].str();
            uint32_t addr = table.at(str);
            if (inst.type == INST_TYPE::J_TYPE) {
                inst.target = addr >> 2;
            } else {
                uint16_t rel_addr = (addr - line.addr - 4) >> 2;  // TODO: negative number?
                inst.imm = rel_addr;
            }
        }
    }
    return inst;
}