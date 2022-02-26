#include <array>
#include <functional>
#include <iostream>
#include <sstream>

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

size_t Intel8080::debug_execute() {
    size_t cycles = 0;
    std::cout << "PC = " << std::hex << (int)PC
              << ", M[PC] = " << (int)memory[PC] << std::endl;
    while (!halted) {
        std::cout << "> ";
        std::string buffer;
        std::getline(std::cin, buffer);
        if (buffer == "h" || buffer == "help") {
            std::cout << "Commands:" << std::endl;
            std::cout << "\tstep (s): Execute the next instruction";
        } else if (buffer == "s" || buffer == "step") {
            cycles += instruction(memory[PC++]);
            std::cout << "PC = " << std::hex << (int)PC
                      << ", M[PC] = " << (int)memory[PC] << std::endl;
        } else {
            std::cout << "Invalid command" << std::endl;
        }
    }
    return cycles;
}

size_t Intel8080::instruction(uint8_t inst) {
    if ((inst & 0xc0) == 0x00) {
        if ((inst & 0x07) == 0x00 && (inst >> 8) < 4) {
            return 4;
        }
        if ((inst & 0xc7) == 0x06) {
            auto dst = (inst >> 3) & 0x7;
            register8(dst) = readByte();
            return dst == 6 ? 10 : 7;
        }
        if ((inst & 0xcf) == 0x01) {
            auto rp = (inst >> 4) & 0x3;
            register16(rp) = readWord();
            return 10;
        }
    } else if ((inst & 0xc0) == 0x40) {
        if (inst == 0x76) {
            halted = true;
            return 7;
        }
        auto dst = (inst >> 3) & 0x7;
        auto src = inst & 0x7;
        register8(dst) = register8(src);
        return (dst == 6 || src == 6) ? 7 : 5;
    } else if ((inst & 0xc0) == 0x80) {
        if ((inst & 0xf8) == 0xa0) {
            auto src = inst & 0x7;
            A = and8(register8(src));
            return (src == 6) ? 7 : 4;
        }
    } else if ((inst & 0xc0) == 0xc0) {
        switch (inst) {
        case 0xc3:
            PC = readWord();
            return 10;
        case 0xd3:
            if (out_callback != nullptr) {
                out_callback(readByte(), A);
            }
            return 10;
        case 0xd8:
            if (in_callback != nullptr) {
                A = in_callback(readByte());
            }
            return 10;
        case 0xfe:
            sub8(readByte());
            return 7;
        }
        if ((inst & 0xc7) == 0xc2) {
            auto ccc = (inst >> 3) & 0x7;
            auto address = readWord();
            if ((ccc == 0 && FLAGS.Z) || (ccc == 1 && !FLAGS.Z) ||
                (ccc == 2 && FLAGS.C) || (ccc == 3 && !FLAGS.C) ||
                (ccc == 4 && FLAGS.P) || (ccc == 5 && !FLAGS.P) ||
                (ccc == 6 && FLAGS.S) || (ccc = 7 && !FLAGS.S)) {
                PC = address;
            }
        }
        return 10;
    }
    std::ostringstream os;
    os << "Unimplemented instruction 0x" << std::hex << (int)inst << std::endl;
    throw std::runtime_error(os.str());
}

uint8_t Intel8080::and8(uint8_t value) {
    uint16_t result = A & value;
    FLAGS.Z = result == 0 ? 1 : 0;
    // FLAGS.C = ???
    // FLAGS.P = TODO
    FLAGS.S = (result & 0x80) ? 1 : 0;
    return result & 0xff;
}

uint8_t Intel8080::sub8(uint8_t value) {
    uint16_t result = A - value;
    FLAGS.Z = result == 0 ? 1 : 0;
    // FLAGS.C = ???
    // FLAGS.P = TODO
    FLAGS.S = (result & 0x80) ? 1 : 0;
    return result & 0xff;
}

uint8_t Intel8080::readByte() { return memory[PC++]; }

uint16_t Intel8080::readWord() {
    uint16_t lb = memory[PC++];
    uint16_t hb = memory[PC++];
    return (hb << 8) | lb;
}

uint8_t &Intel8080::register8(uint8_t code) {
    std::array<std::reference_wrapper<uint8_t>, 8> registers = {B, C, D, E,
                                                                H, L, A, A};
    return (code == 6) ? memory[HL] : registers[code].get();
}

uint16_t &Intel8080::register16(uint8_t code) {
    std::array<std::reference_wrapper<uint16_t>, 4> registers = {BC, DE, HL,
                                                                 SP};
    return registers[code].get();
}