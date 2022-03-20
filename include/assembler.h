#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <istream>
#include <optional>
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

    bool had_errors = false;

  public:
    Intel8080Assembler(std::istream &istream) : is(istream), line_number(0) {}

    std::vector<uint8_t> assemble();
    bool hadErrors() { return had_errors; }

  private:
    void assembleLine(std::vector<uint8_t> &program);
    void fixBranches(std::vector<uint8_t> &program);

    std::vector<std::string> parseLine();

    std::optional<uint16_t> parseAddress(std::string token);
    uint8_t parseImmediate(std::string token);
    uint8_t parseRegister(std::string token);
    uint8_t parseRegisterPair(std::string token);

    template <typename... Ts> void error(Ts... args);
};

#endif