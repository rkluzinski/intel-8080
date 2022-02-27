#include <array>
#include <functional>
#include <iomanip>
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
    while (!halted) {
        std::cout << "PC = " << std::hex << std::setw(4) << (int)PC
                  << ", M[PC] = " << std::setw(2) << (int)memory[PC]
                  << ", A = " << std::setw(2) << (int)A
                  << ", DE = " << std::setw(4) << (int)DE
                  << ", Z = " << std::setw(1) << (int)FLAGS.Z << std::endl;
        cycles += instruction(memory[PC++]);
    }
    return cycles;
}

size_t Intel8080::instruction(uint8_t inst) {
    uint8_t ccc = (inst >> 3) & 0x7;
    uint8_t dst = (inst >> 3) & 0x7;
    uint8_t src = inst & 0x7;
    uint8_t rp = (inst >> 4) & 0x3;
    if ((inst & 0xc0) == 0x00) {
        switch (inst & 0x0f) {
        case 0x00:
        case 0x08:
            // NOP
            return 4;
        case 0x01:
            // LXI
            register16(rp) = readWord();
            return 10;
        case 0x02:
            if (inst == 0x22) {
                // SHLD
            } else if (inst == 0x32) {
                // STA
            }
            // STAX
            memory[register16(rp)] = A;
            return 7;
        case 0x03:
            // INX
            register16(rp) += 1;
            return 5;
        case 0x04:
        case 0x0c:
            // INR
            register8(dst) += 1;
            return dst == 6 ? 10 : 5;
        case 0x06:
        case 0x0e:
            // MVI
            register8(dst) = readByte();
            return dst == 6 ? 10 : 7;
        case 0x0a:
            if (inst == 0x2a) {
                // LHLD
            } else if (inst == 0x3a) {
                // LDA
            }
            // LDAX
            A = memory[register16(rp)];
            return 7;
        case 0x0b:
            // DCX
            register16(rp) -= 1;
            return 5;
        case 0x0f:
            if (inst == 0x0f) {
                // RRC
                auto C = A & 0x1;
                A = (A >> 1) | (C << 8);
                FLAGS.C = C;
            } else if (inst == 0x1f) {
                // RAR
            } else if (inst == 0x2f) {
                // CMA
            } else {
                // CMC
            }
            return 4;
        }
    } else if ((inst & 0xc0) == 0x40) {
        if (inst == 0x76) {
            halted = true;
            return 7;
        }
        register8(dst) = register8(src);
        return (dst == 6 || src == 6) ? 7 : 5;
    } else if ((inst & 0xc0) == 0x80) {
        switch ((inst >> 3) & 0x7) {
        case 0x00:
            A = add(register8(src));
            break;
        case 0x01:
            A = adc(register8(src));
            break;
        case 0x04:
            A = ana(register8(src));
            break;
        }
        return (src == 6) ? 7 : 4;
    } else if ((inst & 0xc0) == 0xc0) {
        switch (inst & 0x0f) {
        case 0x02:
        case 0x0a:
            // Jccc
            if ((ccc == 0 && !FLAGS.Z) || (ccc == 1 && FLAGS.Z) ||
                (ccc == 2 && !FLAGS.C) || (ccc == 3 && FLAGS.C) ||
                (ccc == 4 && !FLAGS.P) || (ccc == 5 && FLAGS.P) ||
                (ccc == 6 && !FLAGS.S) || (ccc == 7 && FLAGS.S)) {
                PC = readWord();
            } else {
                readWord();
            }
            return 10;
        case 0x03:
            if (inst == 0xc3) {
                // JMP
                PC = readWord();
                return 10;
            } else if (inst == 0xd3) {
                // OUT
                if (out_callback != nullptr) {
                    out_callback(readByte(), A);
                }
                return 10;
            } else if (inst == 0xe3) {
                // XTHL
            } else {
                // DI
            }
            break;
        case 0x05:
            pushWord(register16(rp));
            return 11;
        case 0x09:
            if (inst == 0xc9 || inst == 0xd9) {
                PC = popWord();
                return 10;
            } else if (inst == 0xe9) {
                // PCHL
            } else {
                // SPHL
            }
            break;
        case 0x0b:
            if (inst == 0xcb) {
                // JMP
                PC = readWord();
                return 10;
            } else if (inst == 0xdb) {
                // IN
                if (in_callback != nullptr) {
                    A = in_callback(readByte());
                }
                return 10;
            } else if (inst == 0xeb) {
                std::swap(DE, HL);
                return 5;
            } else {
                // EI
            }
            break;
        case 0x0d:
            // CALL
            pushWord(PC);
            PC = readWord();
            return 17;
        case 0x0e:
            if (inst == 0xce) {
            } else if (inst == 0xde) {
            } else if (inst == 0xee) {
            } else {
                sub(readByte());
                return 7;
            }
        }
    }
    std::ostringstream os;
    os << "Unimplemented instruction 0x" << std::hex << (int)inst << std::endl;
    throw std::runtime_error(os.str());
}

uint8_t Intel8080::add(uint8_t value) {
    uint16_t result = A + value;
    FLAGS.C = result > 0xff;
    FLAGS.AC = (A & 0x7) + (value & 0x7) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::adc(uint8_t value) {
    uint16_t result = A + value + FLAGS.C;
    FLAGS.C = result > 0xff;
    FLAGS.AC = (A & 0x7) + (value & 0x7) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::sub(uint8_t value) {
    uint16_t result = A - value;
    FLAGS.C = result <= 0xff;
    FLAGS.AC = (A & 0x7) + (value & 0x7) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::ana(uint8_t value) {
    uint16_t result = A & value;
    FLAGS.C = 0;
    setZPS(result);
    return result & 0xff;
}

void Intel8080::setZPS(uint16_t result) {
    FLAGS.Z = (result == 0) ? 1 : 0;
    FLAGS.P = parity(result);
    FLAGS.S = (result & 0x80) ? 1 : 0;
}

uint8_t Intel8080::parity(uint8_t value) {
    value ^= value << 4;
    value ^= value << 2;
    value ^= value << 1;
    return value & 0x1;
}

void Intel8080::pushWord(uint16_t word) {
    memory[--SP] = word & 0xff;
    memory[--SP] = (word >> 8) & 0xff;
}

uint16_t Intel8080::popWord() {
    uint16_t hb = memory[SP++];
    uint16_t lb = memory[SP++];
    return (hb << 8) | lb;
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