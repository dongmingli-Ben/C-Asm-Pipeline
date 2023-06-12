#ifndef __OPS_CSC3050
#define __OPS_CSC3050

#include <map>
#include "simulator.h"

#define TEXT_OFFSET 0x400000


void register_r_type_functions(std::map<int, int (*)(int, int, int&, Memory&)> &func_map);
void register_other_functions(std::map<int, int (*)(int, int, int&, Memory&)> &func_map);
void register_register_names(std::map<int, std::string> &reg_map);
void register_opcode2name(std::map<int, std::string> &opcode2name);
void register_funct2name(std::map<int, std::string> &funct2name);
#endif