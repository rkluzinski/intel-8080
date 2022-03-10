#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "emulator.h"

void Intel8080::reset() {
    PC = 0;
    SP = 0;
    BC = 0;
    DE = 0;
    HL = 0;
    PSW = 2;
    halted = false;
    interrupts = true;
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
        // PC=%%%%(%%) A=%% SZAPC=%%%%% BC=%%%% DE=%%%% HL=%%%%
        std::cerr << std::hex << std::setfill('0') << "PC=" << std::setw(4)
                  << PC << "[" << std::setw(2) << (int)memory[PC] << "]";
        cycles += instruction(memory[PC++]);
        std::cerr << " A=" << std::setw(2) << (int)A
                  << " SZAPC=" << (int)FLAGS.S << (int)FLAGS.Z << (int)FLAGS.A
                  << (int)FLAGS.P << (int)FLAGS.C << " BC=" << std::setw(4)
                  << BC << " DE=" << std::setw(4) << DE
                  << " HL=" << std::setw(4) << HL << " SP=" << std::setw(4)
                  << SP << "[" << std::setw(2) << (int)memory[SP + 1]
                  << std::setw(2) << (int)memory[SP] << "]" << std::endl;
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
                uint16_t address = readWord();
                memory[address] = L;
                memory[address + 1] = H;
                return 16;
            } else if (inst == 0x32) {
                // STA
                uint16_t address = readWord();
                memory[address] = A;
                return 13;
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
        case 0x05:
        case 0x0d:
            // DCR
            setZPS(--register8(dst));
            return dst == 6 ? 10 : 5;
        case 0x06:
        case 0x0e:
            // MVI
            register8(dst) = readByte();
            return dst == 6 ? 10 : 7;
        case 0x07:
            if (inst == 0x07) {
                // RLC
                FLAGS.C = (A >> 7) & 0x1;
                A = (A << 1) | (A >> 7);
            } else if (inst == 0x17) {
                // RAL
                uint8_t tmp = A << 1;
                tmp |= FLAGS.C;
                FLAGS.C = A >> 7;
                A = tmp;
            } else if (inst == 0x27) {
                // DAA
                if ((A & 0x0f) > 0x09 || FLAGS.A == 1) {
                    FLAGS.A = 1;
                    A += 6;
                } else {
                    FLAGS.A = 0;
                }
                if ((A & 0xf0) > 0x90 || FLAGS.C == 1) {
                    FLAGS.C = 1;
                    A += 0x60;
                } else {
                    FLAGS.C = 0;
                }
                setZPS(A);
            } else {
                // STC
                FLAGS.C = 1;
            }
            return 4;
        case 0x09:
            HL += register16(rp);
            FLAGS.C = HL < register16(rp) ? 1 : 0;
            return 10;
        case 0x0a:
            if (inst == 0x2a) {
                // LHLD
                uint16_t address = readWord();
                L = memory[address];
                H = memory[address + 1];
                return 16;
            } else if (inst == 0x3a) {
                // LDA
                uint16_t address = readWord();
                A = memory[address];
                return 13;
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
                FLAGS.C = A & 0x1;
                A = (A >> 1) | (A << 7);
            } else if (inst == 0x1f) {
                // RAR
                uint8_t tmp = A >> 1;
                tmp |= FLAGS.C << 7;
                FLAGS.C = A & 0x1;
                A = tmp;
            } else if (inst == 0x2f) {
                // CMA
                A = ~A;
            } else {
                // CMC
                FLAGS.C = ~FLAGS.C;
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
        case 0x02:
            A = sub(register8(src));
            break;
        case 0x03:
            A = sbb(register8(src));
            break;
        case 0x04:
            A = ana(register8(src));
            break;
        case 0x05:
            A = xra(register8(src));
            break;
        case 0x06:
            A = ora(register8(src));
            break;
        case 0x07:
            sub(register8(src));
            break;
        }
        return (src == 6) ? 7 : 4;
    } else if ((inst & 0xc0) == 0xc0) {
        switch (inst & 0x0f) {
        case 0x00:
        case 0x08:
            // Rccc
            if ((ccc == 0 && !FLAGS.Z) || (ccc == 1 && FLAGS.Z) ||
                (ccc == 2 && !FLAGS.C) || (ccc == 3 && FLAGS.C) ||
                (ccc == 4 && !FLAGS.P) || (ccc == 5 && FLAGS.P) ||
                (ccc == 6 && !FLAGS.S) || (ccc == 7 && FLAGS.S)) {
                PC = popWord();
                return 11;
            } else {
                return 5;
            }
        case 0x01:
            (rp == 3 ? PSW : register16(rp)) = popWord();
            return 10;
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
                uint16_t tmp = HL;
                L = memory[SP];
                H = memory[SP + 1];
                memory[SP] = tmp;
                memory[SP + 1] = tmp >> 8;
                return 10;
            } else {
                // DI
                interrupts = false;
                return 4;
            }
        case 0x04:
        case 0x0c:
            // Cccc
            if ((ccc == 0 && !FLAGS.Z) || (ccc == 1 && FLAGS.Z) ||
                (ccc == 2 && !FLAGS.C) || (ccc == 3 && FLAGS.C) ||
                (ccc == 4 && !FLAGS.P) || (ccc == 5 && FLAGS.P) ||
                (ccc == 6 && !FLAGS.S) || (ccc == 7 && FLAGS.S)) {
                pushWord(PC + 2);
                PC = readWord();
                return 17;
            } else {
                readWord();
                return 11;
            }
        case 0x05:
            pushWord(rp == 3 ? PSW : register16(rp));
            return 11;
        case 0x06:
            if (inst == 0xc6) {
                A = add(readByte());
            } else if (inst == 0xd6) {
                A = sub(readByte());
            } else if (inst == 0xe6) {
                A = ana(readByte());
            } else {
                A = ora(readByte());
            }
            return 7;
        case 0x09:
            if (inst == 0xc9 || inst == 0xd9) {
                PC = popWord();
                return 10;
            } else if (inst == 0xe9) {
                PC = HL;
                return 5;
            } else {
                SP = HL;
                return 5;
            }
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
                interrupts = true;
                return 4;
            }
        case 0x0d:
            // CALL
            pushWord(PC + 2);
            PC = readWord();
            return 17;
        case 0x0e:
            if (inst == 0xce) {
                A = adc(readByte());
            } else if (inst == 0xde) {
                A = sbb(readByte());
            } else if (inst == 0xee) {
                A = xra(readByte());
            } else {
                sub(readByte());
            }
            return 7;
        }
    }
    std::ostringstream os;
    os << "Unimplemented instruction 0x" << std::hex << (int)inst << std::endl;
    throw std::runtime_error(os.str());
}

uint8_t Intel8080::add(uint8_t value) {
    uint16_t result = A + value;
    FLAGS.C = result > 0xff;
    FLAGS.A = (A & 0xf) + (value & 0xf) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::adc(uint8_t value) {
    uint16_t result = A + value + FLAGS.C;
    FLAGS.C = result > 0xff;
    FLAGS.A = (A & 0xf) + (value & 0xf) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::sub(uint8_t value) {
    uint16_t result = A + ~value + 1;
    FLAGS.C = result > 0xff;
    FLAGS.A = (A & 0xf) + (value & 0xf) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::sbb(uint8_t value) {
    uint16_t result = A + ~(value + FLAGS.C) + 1;
    FLAGS.C = result > 0xff;
    FLAGS.A = (A & 0xf) + (value & 0xf) > 0xf;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::ana(uint8_t value) {
    uint16_t result = A & value;
    FLAGS.C = 0;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::xra(uint8_t value) {
    uint16_t result = A ^ value;
    FLAGS.C = 0;
    setZPS(result);
    return result & 0xff;
}

uint8_t Intel8080::ora(uint8_t value) {
    uint16_t result = A | value;
    FLAGS.C = 0;
    setZPS(result);
    return result & 0xff;
}

void Intel8080::setZPS(uint16_t result) {
    FLAGS.Z = (result & 0xff) ? 0 : 1;
    FLAGS.P = ~parity(result);
    FLAGS.S = (result & 0x80) ? 1 : 0;
}

uint8_t Intel8080::parity(uint8_t value) {
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return value & 0x1;
}

void Intel8080::pushWord(uint16_t word) {
    memory[--SP] = (word >> 8) & 0xff;
    memory[--SP] = word & 0xff;
}

uint16_t Intel8080::popWord() {
    uint16_t lb = memory[SP++];
    uint16_t hb = memory[SP++];
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