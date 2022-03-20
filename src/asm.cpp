#include "assembler.h"
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " INPUT OUTPUT" << std::endl;
        return 1;
    }
    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "Failed to open input file '" << argv[1] << "'"
                  << std::endl;
        return 1;
    }
    Intel8080Assembler assembler(input);
    auto program = assembler.assemble();
    if (assembler.hadErrors()) {
        std::cerr << "'" << argv[1] << "' had errors" << std::endl;
        return 1;
    }
    std::ofstream output(argv[2], std::ios::binary);
    if (!output) {
        std::cerr << "Failed to open output file '" << argv[2] << "'"
                  << std::endl;
        return 1;
    }
    if (!output.write((char *)program.data(), program.size())) {
        std::cerr << "Failed to write to '" << argv[2] << "'" << std::endl;
        return 1;
    }
    return 0;
}