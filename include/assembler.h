#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

class Intel8080Assembler {
  private:
    std::istream &is;
    uint8_t *memory;

    std::string line;
    size_t line_number;

    std::unordered_map<std::string, uint16_t> label2addr;
    std::unordered_map<std::string, uint16_t> addr2fix;

    size_t pc;
    bool had_errors = false;

  public:
    Intel8080Assembler(std::istream &istream, uint8_t *memory_)
        : is(istream), memory(memory_), line_number(0) {
        assemble();
    }

    size_t bytesWritten() { return pc; }
    bool hadErrors() { return had_errors; }

  private:
    void assemble();
    void assembleLine();
    void fixBranches();

    std::vector<std::string> parseLine();

    uint16_t parseAddress(std::string token);
    uint8_t parseImmediate(std::string token);
    uint8_t parseRegister(std::string token);
    uint8_t parseRegisterPair(std::string token);

    void emitByte(uint8_t byte);
    void emitWord(uint16_t word);

    template <typename... Ts> void error(Ts... args);
};

#endif