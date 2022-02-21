#include "assembler.h"
#include <iostream>

int main() {
    uint8_t memory[0x10000];
    Intel8080Assembler assembler(std::cin, memory);
    if (assembler.hadErrors()) {
        return 1;
    }
    if (!std::cout.write((char *)memory, assembler.bytesWritten())) {
        std::cerr << "fatal: failed to write to stdout" << std::endl;
        return 1;
    }
    return 0;
}