#include <iostream>
#include <cstdio>
#include "simulator.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << "Please feed input files according to format, received "
             << argc << " files\n";
        exit(EXIT_FAILURE);
    }
    FILE *f_asm, *f_code, *f_chpt, *f_in, *f_out, *f_err;
    f_asm = fopen(argv[1], "r");
    f_code = fopen(argv[2], "r");
    f_chpt = fopen(argv[3], "r");
    f_in = fopen(argv[4], "r");
    f_out = fopen(argv[5], "w");
    f_err = stdout;
    // execution
    Simulator sim(f_asm, f_code, f_chpt, f_in, f_out, f_err);
    try
    {
        sim.run();
        fclose(f_asm);
        fclose(f_code);
        fclose(f_chpt);
        fclose(f_in);
        fclose(f_out);
    }
    catch(const domain_error& e)
    {
        fclose(f_asm);
        fclose(f_code);
        fclose(f_chpt);
        fclose(f_in);
        fclose(f_out);
        return sim.get_exit_status();
    }
    return 0;
}