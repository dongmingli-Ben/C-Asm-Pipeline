#include <fstream>
#include <iostream>
#include "LabelTable.h"
#include "utils.h"

using namespace std;


LabelTable pass1 (char * filename) {
    ifstream file;
    file.open(filename, ios::in);

    LabelTable table;
    int addr = 0x400000;

    line_struct line;
    while ((line = parse_line(file, addr)).addr != 0) {
        // if (line.label.length() > 0)
        //     cout << "[" << line.addr << "] " << line.inst << " (" << line.label << ")\n";
        // else
        //     cout << "[" << line.addr << "] " << line.inst << "\n";
        if (line.label.length() > 0) {
            if (table.find(line.label) != table.end()) {
                cerr << line.label << 
                    " is already defined at address " 
                    << table[line.label] << endl;
                continue;
            }
            table[line.label] = line.addr;
        }
    }
    file.close();
    return table;
}