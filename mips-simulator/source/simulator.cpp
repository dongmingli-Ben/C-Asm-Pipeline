#include "simulator.h"
#include <string.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include "ops.h"

#include <iostream>

using namespace std;

// #define DISPLAY


Memory::Memory() {
#ifdef DISPLAY
    cout << "Memory constructor called" << endl;
#endif
    data = new int[MEMORY_SIZE/4];
    memset(data, 0, MEMORY_SIZE/4);
}

Memory::~Memory() {
#ifdef DISPLAY
    cout << "Memory destructor called" << endl;
#endif
    delete[] data;
}

int& Memory::operator[](int addr) {
    assert(addr % 4 == 0);
    return data[(addr - TEXT_OFFSET) / 4];
}

int8_t& Memory::at8(int addr) {
    if (addr % 4 != 0) {
        fprintf(stderr, "Accessing byte not aligned to word boundary, please check.\n");
    }
    int8_t *pt;
    pt = (int8_t *) data + (addr - TEXT_OFFSET);
    return *pt;
}

int16_t& Memory::at16(int addr) {
    assert(addr % 2 == 0);
    if (addr % 4 != 0) {
        fprintf(stderr, "Accessing half not aligned to word boundary, please check.\n");
    }
    int16_t *pt;
    pt = (int16_t *) data + (addr - TEXT_OFFSET)/2;
    return *pt;
}

int32_t& Memory::at32(int addr) {
    assert(addr % 4 == 0);
    int32_t *pt;
    pt = (int32_t *) data + (addr - TEXT_OFFSET)/4;
    return *pt;
}



Simulator::Simulator(FILE *f_asm, FILE *f_code, 
            FILE *f_chpt, FILE *f_in, FILE *f_out,
            FILE *f_err) {
    this->f_asm = f_asm;
    this->f_code = f_code;
    this->f_chpt = f_chpt;
    this->f_in = f_in;
    this->f_out = f_out;
    this->f_err = f_err;
    reg = new int[32 + 3];
    // initialize
    memset(reg, 0, 32 + 3);
    heap_size = 0;
    get_pc() = TEXT_OFFSET;
    reg[29] = MEMORY_SIZE + TEXT_OFFSET;
    reg[30] = MEMORY_SIZE + TEXT_OFFSET;
    reg[28] = TEXT_SIZE + GP_OFFSET + TEXT_OFFSET;
    register_r_type_functions(funct_code2op_func);
    register_other_functions(op_code2op_func);
    register_register_names(reg2name);
    register_opcode2name(opcode2name);
    register_funct2name(funct2name);
    exit_status = 0;
};

Simulator::~Simulator() {
    delete[] reg;
}

void Simulator::run() {
    initialize();
    init_checkpoints();
    run_code();
};

void Simulator::initialize() {
    // put .text to memory
    char *line = NULL;
    size_t len = 32;
    int i = TEXT_OFFSET, code;
    while (getline(&line, &len, f_code) != -1) {
        code = (int) strtol(line, NULL, 2);
        mem[i] = code;
        i += 4;
    }
    code_size = i - TEXT_OFFSET;
    // put .data to memory
    int offset = TEXT_SIZE + TEXT_OFFSET;
    len = 100;
    string str;
    bool in_data = false;
    data_struct inst;
    while (getline(&line, &len, f_asm) != -1) {
        int str_len = strlen(line);
        if (line[str_len - 1] == '\n') {
            line[str_len - 1] = '\0';
        }
        str = line;
        str = strip_comment(str);
        // assume
        if (strcmp(str.c_str(), ".data") == 0) {
            in_data = true;
            continue;
        } else if (strcmp(str.c_str(), ".text") == 0) {
            in_data = false;
        }
        if (in_data && str.length() > 0) {
            inst = parse_data_instruction(str);
            offset = prepare_data_by_instruction(inst, offset);
        }
    }
    static_size = offset - (TEXT_SIZE + TEXT_OFFSET);
}


void Simulator::run_code() {
    int line = 0, dump_index = 0;
    while (true) {
        dump_checkpoint(line);
#ifdef DISPLAY
        display(stdout);
#endif
        if (get_pc() >= code_size + TEXT_OFFSET) {
            break;
        }
        int inst = fetch_code();
        get_pc() += 4;
        execute_code(inst);
        line++;
    }
}


int Simulator::fetch_code() {
    return mem[get_pc()];
}


void Simulator::execute_code(int code) {
    // handle special instructions, e.g. syscall and output more than two values
    if (code == 12) {
        execute_syscall();
        return;
    } 
    if ((code >> 26) == 0) {
        switch (code & 0x3f) {
            case 0x1a: execute_div(code); return;
            case 0x1b: execute_divu(code); return;
            case 0x18: execute_mult(code); return;
            case 0x19: execute_multu(code); return;
        }
    }
    switch (code >> 26)
    {
        case 0x01:
            if (((code >> 16) & 0x1f) == 0) {
                execute_bltz(code);
            } else if (((code >> 16) & 0x1f) == 1) {
                execute_bgez(code);
            } else {
                fprintf(f_err, "rt %d not valid for opcode %d\n", 
                    (code >> 16) & 0x1f, code >> 26);
                throw invalid_argument("Invalid rt for bgez or bltz");
            }
            return;
        case 0x04: execute_beq(code); return;
        case 0x05: execute_bne(code); return;
        case 0x22: execute_lwl(code); return;
        case 0x26: execute_lwr(code); return;
        case 0x2a: execute_swl(code); return;
        case 0x2e: execute_swr(code); return;
    }
    op_struct op = parse_code(code);
    int in1, in2, out;
    in1 = get_value_from_struct(op.in1);
    in2 = op.in2.type != location::UNK ? get_value_from_struct(op.in2) : -1;
    out = op.op_func(in1, in2, get_pc(), mem);
    if (op.out.type == location::REGISTER
            || op.out.type == location::MEMORY) {
        write_value_by_struct(op.out, out);
    }
}


int Simulator::get_value_from_struct(value_struct v_struct) {
    if (v_struct.type == location::REGISTER) {
        return reg[v_struct.reg];
    } else if (v_struct.type == location::MEMORY) {
        int tgt = reg[v_struct.base] + v_struct.offset;
        assert(tgt % 4 == 0);
        return mem.at32(tgt);
    } else if (v_struct.type == location::IMM) {
        return v_struct.imm;
    }
    fprintf(f_err, "Unknown location to load value\n");
    throw runtime_error("Unknown location to load value");
}


void Simulator::write_value_by_struct(value_struct v_struct, int value) {
    if (v_struct.type == location::REGISTER) {
        reg[v_struct.reg] = value;
    } else if (v_struct.type == location::MEMORY) {
        int tgt = reg[v_struct.base] + v_struct.offset;
        switch (v_struct.size)
        {
        case 1:
            {
                mem.at8(tgt) = value;
                break;
            }
        case 2:
            {
                mem.at16(tgt) = value;
                break;
            }
        case 4:
            {
                mem.at32(tgt) = value;
                break;
            }
        
        default:
            fprintf(f_err, "Unknown size to write value\n");
            throw runtime_error("Unknown size to write value");
            break;
        }
    } else {
        fprintf(f_err, "Unknown location to write value\n");
        throw runtime_error("Unknown location to write value");
    }
}


void Simulator::init_checkpoints() {
    int tmp, i = 0;
    while(fscanf(f_chpt, "%d", &tmp) != EOF){
        checkpoints.insert(tmp);
    }
}


void Simulator::dump_checkpoint(int line) {
    checkpoint_memory(line);
    checkpoint_register(line);
}


void Simulator::checkpoint_memory(int ins_count) {
    if (!checkpoints.count(ins_count))
        return;
    std::string name = "memory_" + std::to_string(ins_count) + ".bin";
    FILE * fp = fopen(name.c_str(), "wb");
    fwrite(mem.data, 4, MEMORY_SIZE/4, fp);
    fclose(fp);
}

void Simulator::checkpoint_register(int ins_count) {
    if (!checkpoints.count(ins_count))
        return;
    std::string name = "register_" + std::to_string(ins_count) + ".bin";
    FILE * fp = fopen(name.c_str(), "wb");
    fwrite(reg, 4, 32+3, fp);
    fclose(fp);
}


/*
If it is a empty line (i.e comment), the size attribute of the
returned data_struct is set to 0. In this case, no action is needed.
*/
data_struct Simulator::parse_data_instruction(string str) {
    data_struct result {.size = 0};
    int index = str.find(':');
    if (index != string::npos) {
        str = str.substr(index+1);
    }
    index = str.find_first_not_of(' ');
    str = str.substr(index);
    if (str[0] == '#') {
        // skip comments
        return result;
    }
    string inst, arg;
    index = str.find_first_of(' ');
    inst = str.substr(0, index);
    str = str.substr(index);
    index = str.find_first_not_of(' ');
    arg = str.substr(index);
    result.instruction = inst;
    // .ascii
    if (inst == ".ascii"s) {
        arg = extract_string_from_arg(arg);
        prepare_string(arg, result, false);
    } else if (inst == ".asciiz"s) {
        arg = extract_string_from_arg(arg);
        prepare_string(arg, result, true);
    } else {
        vector<int> args;
        args = extract_ints_from_args(arg);
        int size;
        if (inst == ".word"s) {
            size = 4;
        } else if (inst == ".half"s) {
            size = 2;
        } else if (inst == ".byte"s) {
            size = 1;
        } else {
            fputs("encounter invalid data type\n", f_err);
            throw runtime_error("encounter invalid data type");
        }
        prepare_ints(args, result, size);
    }
    return result;
}

// return offset by bytes
int Simulator::prepare_data_by_instruction(data_struct inst, int offset) {
    if (inst.size == 0) {
        return offset;
    }
    int n_words = inst.size/4 + (bool) (inst.size%4);
    for (int i = 0; i < n_words; i++) {
        mem[offset+i*4] = inst.data[i];
    }
    offset += n_words * 4;
    delete[] inst.data;
    return offset;
}


string Simulator::extract_string_from_arg(string arg) {
    int index;
    index = arg.find('"');
    if (index == string::npos) {
        fprintf(f_err, "Cannot extract string from .ascii/.asciiz, found %s", arg);
        throw runtime_error(".data invalid format for .ascii/.asciiz");
    }
    arg = arg.substr(index+1);
    index = arg.find('"');
    if (index == string::npos) {
        fprintf(f_err, "\" is not closed by \"\n");
        throw runtime_error("\" is not closed by \"");
    }
    string content = arg.substr(0, index);
    arg = arg.substr(index+1);
    index = arg.find_first_not_of(' ');
    if (index != string::npos) {
        fprintf(f_err, "found text after .ascii/.asciiz statment, %s\n", arg);
        throw runtime_error("invalid .data format: text after .ascii/.asciiz");
    }
    // parse special char in arg, such as \t, \n
    string res;
    for (int i = 0; i < content.size(); i++) {
        if (content[i] != '\\') {
            res.push_back(content[i]);
            continue;
        }
        if (i+1 >= content.size()) {
            fprintf(f_err, "string ends with \\ is not valid, found %s\n", content);
            throw runtime_error(".data invalid format for special character \\");
        }
        if (content[i+1] == 'n') {
            res.push_back('\n');
        } else if (content[i+1] == '0') {
            res.push_back('\0');
        } else if (content[i+1] == '"') {
            res.push_back('"');
        } else if (content[i+1] == '\\') {
            res.push_back('\\');
        } else {
            // otherwise, just 2 regular characters
            res.push_back(content[i]);
            res.push_back(content[i+1]);
        }
        i += 1;
    }
    return res;
}


void Simulator::prepare_string(string arg, data_struct& result, bool null_end) {
    int n_words, n_bytes;
    n_bytes = null_end ? arg.size() + 1 : arg.size();
    n_words = n_bytes / 4 + (bool) (n_bytes % 4);
    result.data = new int[n_words];
    memset(result.data, 0, n_words*4);
    char *pt = (char *) result.data;
    for (int i = 0; i < arg.length(); i++) {
        pt[i] = arg[i];
    }
    result.size = n_bytes;
}


vector<int> Simulator::extract_ints_from_args(string arg) {
    vector<int> content;
    int index;
    index = arg.find_first_not_of(' ');
    if (index == string::npos) {
        fprintf(f_err, "expect to read integers, found empty string %s\n", arg);
        throw runtime_error("cannot read integers");
    }
    arg = arg.substr(index);
    while (arg.length() > 0) {
        index = arg.find(',');
        int x = atoi(arg.substr(0, index).c_str());
        content.push_back(x);
        if (index == string::npos) {
            break;
        }
        arg = arg.substr(index+1);
        index = arg.find_first_not_of(' ');
        arg = arg.substr(index);
    }
    return content;
}

/*
Little Endian !!!
*/
void Simulator::prepare_ints(const vector<int>& args, data_struct& result, int size) {
    result.size = size * args.size();
    int n_words = size/4 + (bool) (size%4);
    result.data = new int[n_words];
    int *ip = result.data;
    int16_t *sp = (int16_t *) result.data;
    int8_t *cp = (int8_t *) result.data;
    for (int i = 0; i < args.size(); i++) {
        if (size == 4) {
            ip[i] = args[i];
        } else if (size == 2) {
            // int tgt = i^1;
            sp[i] = (int16_t) args[i];
        } else if (size == 1) {
            // int j = i & 3;
            // int tgt = (i >> 2 << 2) | (3-j);
            cp[i] = (int8_t) args[i];
        } else {
            fprintf(f_err, "other size is not allowed, found %d\n", size);
            throw invalid_argument("size not allowed");
        }
    }
}


int& Simulator::get_pc() {
    return reg[PC_IDX];
}

string Simulator::strip_comment(string str) {
    int index;
    index = str.find('#');
    if (index == string::npos) {
        return str;
    }
    str = str.substr(0, index);
    index = str.find_last_not_of(" \t\n");
    if (index == string::npos) {
        return ""s;
    }
    str = str.substr(0, index+1);
    index = str.find_first_not_of(" \t\n");
    str = str.substr(index);
    return str;
}

int Simulator::get_exit_status() {
    return exit_status;
}