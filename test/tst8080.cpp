#include <fstream>
#include <iostream>
#include <string>

#include "emulator.h"

void load_binary(Intel8080 &i8080, std::string filename, uint16_t address) {
    auto flags = std::ios::in | std::ios::binary | std::ios::ate;
    std::ifstream file(filename, flags);
    if (file.is_open()) {
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        auto *start = reinterpret_cast<char *>(i8080.memory.data() + address);
        file.read(start, size);
        file.close();
    } else {
        throw std::runtime_error("Could not open '" + filename + "'");
    }
}

uint8_t in_callback(uint8_t port) {
    static uint8_t halt = 0;
    if (port == 0) {
        return halt++;
    }
    return 0;
}

void out_callback(uint8_t port, uint8_t A) {
    if (port == 0) {
        std::cout << *reinterpret_cast<char *>(&A);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "usage: tst8080 OS_BIN TEST_COM" << std::endl;
        return 0;
    }
    try {
        Intel8080 i8080;
        load_binary(i8080, argv[1], 0x0);
        load_binary(i8080, argv[2], 0x100);
        i8080.in_callback = in_callback;
        i8080.out_callback = out_callback;
        i8080.debug_execute();
        std::cout << std::endl;
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}