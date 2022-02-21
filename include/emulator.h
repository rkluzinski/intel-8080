#ifndef EMULATOR_H
#define EMULATOR_H

#include <bit>
#include <cstddef>
#include <cstdint>

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

#endif