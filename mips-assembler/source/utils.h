#ifndef __instruction_util
#define __instruction_util

#include "LabelTable.h"
#include <string>
#include <fstream>


struct line_struct {
    std::string inst;
    int addr;
    std::string label;
};


/*
Parse the current line.

Only remove comments, and annotate labels.
Also increment `cur_addr` by 4.
*/
line_struct parse_line(std::ifstream & stream, int & cur_addr);


std::string trim_left_space(std::string s);
std::string trim_right_space(std::string s);
std::string trim_comment(std::string s);

std::string int_to_binary_str(std::uint32_t code);


#endif