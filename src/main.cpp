#include <bit>
#include <cstdint>
#include <iostream>

static_assert(std::endian::native == std::endian::little);

#define RegisterPair(X, Y) \
    union                  \
    {                      \
        struct             \
        {                  \
            uint8_t Y;     \
            uint8_t X;     \
        };                 \
        uint16_t X##Y;     \
    }

struct Intel8080
{
    uint16_t PC;
    uint16_t SP;
    RegisterPair(B, C);
    RegisterPair(D, E);
    RegisterPair(H, L);
    union
    {
        struct
        {
            uint8_t A;
            struct
            {
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

    bool halt;
    uint8_t memory[0x10000];

    Intel8080() : PC(0), SP(0xffff), BC(0), DE(0), HL(0), PSW(0x0200) {}

    void execute() noexcept;

private:
    template <uint8_t INST>
    void instruction() noexcept;

    constexpr uint8_t &decode_register(uint8_t register_code) noexcept;
};

void Intel8080::execute() noexcept
{
    halt = false;
    while (!halt)
    {
        std::cout << std::hex << (int)memory[PC] << std::endl;
        switch (memory[PC++])
        {
        case 0x76:
            instruction<0x76>();
            break;
        case 0x78:
            instruction<0x78>();
            break;
        }
    }
}

template <uint8_t INST>
void Intel8080::instruction() noexcept
{
    if constexpr ((INST & 0xc0) == 0x00)
    {
        if constexpr (INST == 0x00)
            return;
    }
    if constexpr ((INST & 0xc0) == 0x40)
    {
        if constexpr (INST == 0x76)
        {
            halt = true;
            return;
        }

        decode_register((INST >> 3) & 0x7) = decode_register(INST & 0x7);
    }
    if constexpr ((INST & 0xc0) == 0x80)
    {
    }
    if constexpr ((INST & 0xc0) == 0xc0)
    {
    }
}

constexpr uint8_t &Intel8080::decode_register(uint8_t register_code) noexcept
{
    switch (register_code)
    {
    case 0:
        return B;
    case 1:
        return C;
    case 2:
        return D;
    case 3:
        return E;
    case 4:
        return H;
    case 5:
        return L;
    case 6:
        return memory[HL];
    case 7:
    default:
        return A;
    }
}

int main()
{
    Intel8080 i8080;
    i8080.B = 0xab;
    i8080.memory[0] = 0x78; // MOV A,B
    i8080.memory[1] = 0x76; // HLT
    i8080.execute();
    std::cout << std::hex << "0x" << (int)i8080.A << std::endl;
    return 0;
}