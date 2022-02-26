#ifndef EMULATOR_H
#define EMULATOR_H

#include <array>
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

    std::array<uint8_t, 0x10000> memory;

    using InCallback = uint8_t(uint8_t);
    InCallback *in_callback = nullptr;

    using OutCallback = void(uint8_t, uint8_t);
    OutCallback *out_callback = nullptr;

    Intel8080() { reset(); }

    void reset();

    size_t execute(size_t cycle_limit);
    size_t execute();

    size_t debug_execute();

  private:
    size_t instruction(uint8_t inst);

    uint8_t and8(uint8_t value);
    uint8_t sub8(uint8_t value);

    uint8_t readByte();
    uint16_t readWord();

    uint8_t &register8(uint8_t code);
    uint16_t &register16(uint8_t code);
};

#endif