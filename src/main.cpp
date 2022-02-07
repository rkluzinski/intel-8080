#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <iostream>

static_assert(std::endian::native == std::endian::little);

#define RegisterPair(X, Y)                                                                                             \
    union {                                                                                                            \
        struct {                                                                                                       \
            uint8_t Y;                                                                                                 \
            uint8_t X;                                                                                                 \
        };                                                                                                             \
        uint16_t X##Y;                                                                                                 \
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

    Intel8080() : PC(0), SP(0xffff), BC(0), DE(0), HL(0), PSW(0x0200) {}

    void execute() noexcept;

  private:
    void dispatch(uint8_t inst) noexcept;

    using DispatchTable = std::array<void (Intel8080::*)(), 256>;
    static constexpr DispatchTable build_dispatch_table() noexcept;

    template <uint8_t INST> void instruction() noexcept;

    constexpr uint8_t &register8(uint8_t code) noexcept;
    constexpr uint16_t &register16(uint8_t code) noexcept;
};

void Intel8080::execute() noexcept {
    halted = false;
    while (!halted) {
        std::cout << std::hex << (int)memory[PC] << std::endl;
        dispatch(memory[PC++]);
    }
}

template <size_t i, size_t end, typename F> constexpr void constexpr_for(F &&f) {
    if constexpr (i < end) {
        f(std::integral_constant<decltype(i), i>());
        constexpr_for<i + 1, end>(f);
    }
}

constexpr Intel8080::DispatchTable Intel8080::build_dispatch_table() noexcept {
    Intel8080::DispatchTable table{};
    constexpr_for<0, 256>([&table](auto i) { table[i] = &Intel8080::instruction<i>; });
    return table;
}

void Intel8080::dispatch(uint8_t inst) noexcept {
    static constexpr auto dispatch_table = build_dispatch_table();
    (*this.*dispatch_table[inst])();
}

template <uint8_t INST> void Intel8080::instruction() noexcept {
    if constexpr ((INST & 0xc0) == 0x00) {
        if constexpr (INST == 0x00) {
            return;
        }
    } else if constexpr ((INST & 0xc0) == 0x40) {
        if constexpr (INST == 0x76) {
            halted = true;
            return;
        }
        register8((INST >> 3) & 0x7) = register8(INST & 0x7);
    } else if constexpr ((INST & 0xc0) == 0x80) {
    } else if constexpr ((INST & 0xc0) == 0xc0) {
    }
}

constexpr uint8_t &Intel8080::register8(uint8_t code) noexcept {
    std::array<std::reference_wrapper<uint8_t>, 8> registers = {B, C, D, E, H, L, A, A};
    return (code == 6) ? memory[HL] : registers[code].get();
}

int main() {
    Intel8080 i8080;
    i8080.B = 0xab;
    i8080.memory[0] = 0x78; // MOV A,B
    i8080.memory[1] = 0x76; // HLT
    i8080.execute();
    std::cout << std::hex << "0x" << (int)i8080.A << std::endl;
    return 0;
}