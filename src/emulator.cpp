#include <array>
#include <functional>
#include <iostream>

#include "emulator.h"

void Intel8080::reset() {
    PC = 0;
    SP = 0xffff;
    halted = false;
}

size_t Intel8080::execute(size_t cycle_limit) {
    size_t cycles = 0;
    while (!halted && cycles < cycle_limit) {
        cycles += instruction(memory[PC++]);
    }
    return cycles;
}

size_t Intel8080::execute() {
    size_t cycles = 0;
    while (!halted) {
        cycles += instruction(memory[PC++]);
    }
    return cycles;
}

size_t Intel8080::instruction(uint8_t inst) {
    if ((inst & 0xc0) == 0x00) {
        if ((inst & 0x7) == 0x0 && (inst >> 8) < 4) {
            return 4;
        }
    } else if ((inst & 0xc0) == 0x40) {
        if (inst == 0x76) {
            halted = true;
            return 7;
        }
        auto dst = (inst >> 3) & 0x7;
        auto src = inst & 0x7;
        register8(dst) = register8(src);
        if (dst == 6 || src == 6) {
            return 7;
        } else {
            return 5;
        }
    } else if ((inst & 0xc0) == 0x80) {
    } else if ((inst & 0xc0) == 0xc0) {
    }
    return 0;
}

uint8_t &Intel8080::register8(uint8_t code) {
    std::array<std::reference_wrapper<uint8_t>, 8> registers = {B, C, D, E,
                                                                H, L, A, A};
    return (code == 6) ? memory[HL] : registers[code].get();
}