#include "utils.h"
#include <bitset>

using namespace std;

string trim_left_space(string s) {
    if (s.length() == 0) return s;
    int i = s.find_first_not_of(' ');
    if (i == s.npos) return s;
    s = s.substr(i);
    i = s.find_first_not_of('\t');
    if (i == s.npos) return s;
    s = s.substr(i);
    return s;
}

string trim_right_space(string s) {
    if (s.length() == 0) return s;
    int i = s.find_last_not_of(' ');
    if (i == s.npos) return s;
    s = s.substr(0, i+1);
    i = s.find_last_not_of('\t');
    if (i == s.npos) return s;
    s = s.substr(0, i+1);
    return s;
}

string trim_comment(string s) {
    if (s.length() == 0) return s;
    int i = s.find_first_of('#');
    if (i != s.npos)
        s = s.substr(0, i);
    s = trim_left_space(s);
    s = trim_right_space(s);
    return s;
}


line_struct parse_line(ifstream & stream, int & cur_addr) {
    line_struct line;
    line.addr = 0;

    bool inst_found = false;
    static bool is_code = false;
    char str[100];
    int i;
    string content, label;
    while (inst_found != true) {
        stream.getline(str, 99);
        content = string(str);
        if (stream.eof() && content.length() == 0) {
            // cerr << "instruction is not complete at " << cur_addr << endl;
            // exit(EXIT_FAILURE);
            return line;
        }
        content = trim_left_space(content);
        while (content.substr(0, 5) == ".text"s || content.substr(0, 5) == ".data"s) {
            if (content.substr(0, 5) == ".text"s) {
                is_code = true;
            } else {
                is_code = false;
            }
            content = content.substr(5);
            content = trim_left_space(content);
        }
        if (!is_code) continue;
        // parse .text section
        // remove comment first
        content = trim_comment(content);
        // parse label
        // 1. single line label
        if (content.length() == 0) continue;
        i = content.find_first_of(':');
        if (i != content.npos) {
            label = content.substr(0, i);
            content = content.substr(i+1);
            content = trim_left_space(content);
        }
        if (content.length() == 0) continue;
        // remaining content is the instruction
        line.inst = content;
        line.addr = cur_addr;
        line.label = label;
        inst_found = true;
    }
    cur_addr += 4;
    return line;
}


string int_to_binary_str(uint32_t code) {
    string s = bitset<32>(code).to_string();
    return s;
}