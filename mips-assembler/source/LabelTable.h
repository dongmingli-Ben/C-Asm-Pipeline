#ifndef __LabalTable
#define __LabalTable

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string, int> LabelTable;


LabelTable pass1 (char * filename);
LabelTable pass2 (char * filename, LabelTable table);


#endif