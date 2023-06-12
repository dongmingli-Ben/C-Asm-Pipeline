#ifndef __SIMULATOR_CSC3050
#define __SIMULATOR_CSC3050
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <map>

#define MEMORY_SIZE 0x600000
#define TEXT_SIZE 0x100000
#define GP_OFFSET 0x8000
#define TEXT_OFFSET 0x400000
#define PC_IDX 32
#define HIGH_REG_IDX 33
#define LOW_REG_IDX 34
#define A0_IDX 4
#define A1_IDX 5
#define A2_IDX 6
#define A3_IDX 7
#define V0_IDX 2

#define DISPLAY_STATIC_SIZE 96
#define DISPLAY_HEAP_SIZE 32
#define DISPLAY_STACK_SIZE 32
#define DISPLAY_MEM_PER_ROW 8


struct data_struct {
    std::string instruction;
    int *data; // data should be aligned and in their desired size
    int size; // in bytes
};

enum class location {
    REGISTER, MEMORY, IMM, UNK // UNK for not writing
};

struct value_struct {
    int reg = -1;
    int base = -1;
    int offset = -1;
    int imm;
    int size = -1; // for save instruction
    location type;
};

class Memory {
public:
    Memory();
    ~Memory();

    // address in bytes
    int& operator[](int addr);
    int8_t& at8(int addr);
    int16_t& at16(int addr);
    int32_t& at32(int addr);

    friend class Simulator;

private:
    int *data;
};

struct op_struct {
    value_struct in1 {.type = location::UNK}, 
                 in2 {.type = location::UNK}, 
                 out {.type = location::UNK};
    int (*op_func)(int, int, int&, Memory&);
};

class Simulator {
public:
    Simulator(FILE *f_asm, FILE *f_code, 
            FILE *f_chpt, FILE *f_in, FILE *f_out,
            FILE *f_err);
    ~Simulator();

    void run();
    void initialize();
    void run_code();
    void init_checkpoints();

    std::string strip_comment(std::string str);

    data_struct parse_data_instruction(std::string inst);
    int prepare_data_by_instruction(data_struct inst, int offset);

    std::string extract_string_from_arg(std::string arg);
    void prepare_string(std::string arg, data_struct& result, bool null_end);

    std::vector<int> extract_ints_from_args(std::string arg);
    void prepare_ints(const std::vector<int>& args, data_struct& result, int size);

    int& get_pc();

    void dump_checkpoint(int line);
    void checkpoint_memory(int ins_count);
    void checkpoint_register(int ins_count);
    int fetch_code();
    void execute_code(int code);

    op_struct parse_code(uint32_t code);
    int get_value_from_struct(value_struct v_struct);
    void write_value_by_struct(value_struct v_struct, int value);

    void execute_syscall();
    void execute_div(int code);
    void execute_divu(int code);
    void execute_mult(int code);
    void execute_multu(int code);
    void execute_bltz(int code);
    void execute_bgez(int code);
    void execute_beq(int code);
    void execute_bne(int code);
    void execute_lwl(int code);
    void execute_lwr(int code);
    void execute_swl(int code);
    void execute_swr(int code);

    op_struct parse_r_type(uint32_t code);
    op_struct parse_i_type(uint32_t code);
    op_struct parse_j_type(uint32_t code);

    // helper function for easier debug
    void display(FILE *fp);
    std::string translate_code_to_asm(int code);
    void display_code(FILE *fp);
    void display_registers(FILE *fp);
    void display_data_block(FILE *fp, int start_addr, int n_bytes);
    void display_static_data(FILE *fp, int n_bytes);
    void display_heap_data(FILE *fp, int n_bytes);
    void display_stack_data(FILE *fp, int n_bytes);

    int get_exit_status(void);

    // signed extension
    std::string int2hex(int addr, int size);
    std::string int2binary(int code);

private:
    FILE *f_asm, *f_code, *f_chpt, *f_in, *f_out, *f_err;
    // point to simulated memory
    Memory mem;
    // point to array of simulated registers
    int *reg;
    // size of dynamic data section (in byte)
    int heap_size;
    // size of static data section (in byte)
    int static_size;
    // size of .text (number of instructions) in bytes
    int code_size;
    // exit status for syscall (exit and exit2)
    int exit_status;
    // for dumping checkpoints
    std::set<int> checkpoints;
    // op_func table, the third argument is the target address of PC
    std::map<int, int (*)(int, int, int&, Memory&)> funct_code2op_func;
    std::map<int, int (*)(int, int, int&, Memory&)> op_code2op_func;
    // register id to name
    std::map<int, std::string> reg2name;
    // opcode to name
    std::map<int, std::string> opcode2name;
    // funct to name
    std::map<int, std::string> funct2name;
};


#endif