#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <iostream>

static_assert(std::endian::native == std::endian::little);

#define RegisterPair(X, Y)                                                     \
    union {                                                                    \
        struct {                                                               \
            uint8_t Y;                                                         \
            uint8_t X;                                                         \
        };                                                                     \
        uint16_t X##Y;                                                         \
    }

struct Intel8080 {
    uint16_t PC;
    uint16_t SP;
    RegisterPair(B, C);
    RegisterPair(D, E);
    RegisterPair(H, L);
    union {
        struct {
            uint8_t A;
            struct {
                uint8_t C : 1;
                uint8_t : 1;
                uint8_t P : 1;
                uint8_t : 1;
                uint8_t AC : 1;
                uint8_t : 1;
                uint8_t Z : 1;
                uint8_t S : 1;
            } FLAGS;
        };
        uint16_t PSW;
    };

    bool halted;

    uint8_t memory[0x10000];

    Intel8080() : BC(0), DE(0), HL(0), PSW(0x0000) {
        // TODO fix initial PSW values
        reset();
    }

    void reset();

    size_t execute(size_t cycle_limit);
    size_t execute();

  private:
    size_t instruction(uint8_t inst);

    uint8_t &register8(uint8_t code);
    uint16_t &register16(uint8_t code);
};

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

int main() {
    Intel8080 i8080;
    i8080.B = 0xab;
    i8080.memory[0] = 0x00; // NOP
    i8080.memory[1] = 0x10; // ...
    i8080.memory[2] = 0x20;
    i8080.memory[3] = 0x30;
    i8080.memory[4] = 0x08;
    i8080.memory[5] = 0x18;
    i8080.memory[6] = 0x28;
    i8080.memory[7] = 0x08;
    i8080.memory[8] = 0x78; // MOV A,B
    i8080.memory[9] = 0x76; // HLT
    i8080.execute();
    std::cout << std::hex << "0x" << (int)i8080.A << std::endl;
    return 0;
}