#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "assembler.h"

void Intel8080Assembler::assemble() {
    while (!is.eof()) {
        try {
            assembleLine();
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    fixBranches();
}

void Intel8080Assembler::assembleLine() {
    std::vector<std::string> tokens = parseLine();

    auto it = tokens.cbegin();
    if (it == tokens.cend()) {
        return;
    }

    if (it->back() == ':') {
        auto label = it->substr(0, it->size() - 1);
        if (!label2addr.emplace(label, pc).second) {
            error("label '", label, "' already defined");
        }
        it++;
        if (it == tokens.cend()) {
            error("expected instruction after label '", label, "'");
        }
    }

    auto code = *it++;

    std::string operand1, operand2;
    size_t operand_count = 0;
    if (it != tokens.cend()) {
        operand1 = *it++;
        operand_count++;
        if (it != tokens.cend()) {
            operand2 = *it++;
            operand_count++;
        }
    }

    if (it != tokens.cend()) {
        error("unexpected token '", *it, "' after operand");
    }

    auto assertOperandCountIs = [&, this](size_t expected) {
        if (operand_count != expected) {
            error("opcode '", code, "' expects ", expected, " operands");
        }
    };

    if (code == "cpi") {
        assertOperandCountIs(1);
        emitByte(0xfe);
        emitByte(parseImmediate(operand1));
    } else if (code == "hlt") {
        assertOperandCountIs(0);
        emitByte(0x76);
    } else if (code == "in") {
        assertOperandCountIs(1);
        emitByte(0xd8);
        emitByte(parseImmediate(operand1));
    } else if (code == "inx") {
        assertOperandCountIs(1);
        uint8_t rp = parseRegisterPair(operand1);
        emitByte(0x03 | (rp << 4));
    } else if (code == "jmp") {
        assertOperandCountIs(1);
        emitByte(0xc3);
        emitWord(parseAddress(operand1));
    } else if (code == "mov") {
        assertOperandCountIs(2);
        uint8_t dst = parseRegister(operand1);
        uint8_t src = parseRegister(operand2);
        if (dst == 6 && src == 6) {
            error("invalid operands for opcode 'mov'");
        }
        emitByte(0x40 | (dst << 3) | src);
    } else if (code == "nop") {
        assertOperandCountIs(0);
        emitByte(0x00);
    } else if (code == "out") {
        assertOperandCountIs(1);
        emitByte(0xd3);
        emitByte(parseImmediate(operand1));
    } else if (code == "ret") {
        assertOperandCountIs(0);
        emitByte(0xc5);
    } else if (code == "xchg") {
        assertOperandCountIs(0);
        emitByte(0xeb);
    } else {
        if (code[0] == 'j') {
            auto condition = code.substr(1, code.size() - 1);
            static const std::unordered_map<std::string, uint8_t> conditions = {
                {"nz", 0}, {"z", 1},  {"nc", 2}, {"c", 3},
                {"po", 4}, {"pe", 5}, {"p", 6},  {"m", 7},
            };
            auto it = conditions.find(condition);
            if (it == conditions.end()) {
                error("invalid condition '", condition, "'");
            }
            uint8_t ccc = it->second;
            if (code[0] == 'j') {
                assertOperandCountIs(1);
                emitByte(0xc2 | (ccc << 3));
                emitWord(parseAddress(operand1));
                return;
            }
        }
        error("invalid opcode '", code, "'");
    }
}

void Intel8080Assembler::fixBranches() {
    for (const auto [label, loc] : addr2fix) {
        auto it = label2addr.find(label);
        if (it == label2addr.end()) {
            std::cerr << "error: could not resolve label '" << label << "'"
                      << std::endl;
        } else {
            auto address = it->second;
            memory[loc] = address & 0xff;
            memory[loc + 1] = (address >> 8) & 0xff;
        }
    }
}

std::vector<std::string> Intel8080Assembler::parseLine() {
    static const auto whitespace = std::string(" \t\v\f\r");

    line_number++;
    std::getline(is, line, '\n');
    for (auto &c : line) {
        c = std::tolower(c);
    }

    size_t start = line.find_first_not_of(whitespace, 0);
    size_t end = line.find_first_of(whitespace, start);

    std::vector<std::string> tokens;
    if (start == std::string::npos) {
        return tokens;
    }

    if (line[end - 1] == ':') {
        tokens.push_back(line.substr(start, end - start));
    } else {
        end = 0;
    }

    start = line.find_first_not_of(whitespace, end);
    end = line.find_first_of(whitespace, start);

    if (start == std::string::npos) {
        error("expected instruction after label");
    } else {
        tokens.push_back(line.substr(start, end - start));
    }

    start = line.find_first_not_of(whitespace, end);
    if (start == std::string::npos || line[start] == ';') {
        return tokens;
    }

    end = line.find_first_of(whitespace + ",", start);
    tokens.push_back(line.substr(start, end - start));

    if (line[end] == ',') {
        start = line.find_first_not_of(whitespace, end + 1);
        end = line.find_first_of(whitespace, start);
        tokens.push_back(line.substr(start, end - start));
    }

    start = line.find_first_not_of(whitespace, end);
    if (start != std::string::npos && line[start] != ';') {
        error("expected comment or end of line");
    }

    return tokens;
}

uint16_t Intel8080Assembler::parseAddress(std::string token) {
    if ('0' < token[0] && token[0] < '9') {
        try {
            int base = 10;
            switch (token.back()) {
            case 'H':
                base = 16;
                break;
            case 'O':
                base = 8;
                break;
            case 'B':
                base = 2;
                break;
            }
            int imm = std::stoi(token, nullptr, base);
            if (imm < 0 || imm > 255) {
                throw std::out_of_range("");
            }
            return static_cast<uint8_t>(imm);
        } catch (std::invalid_argument &e) {
            error("could not parse immediate value");
        } catch (std::out_of_range &e) {
            error("immedate value is out of range (0-255)");
        }
    }

    auto it = label2addr.find(token);
    if (it == label2addr.end()) {
        addr2fix.emplace(token, pc);
        return 0;
    }
    return it->second;
}

uint8_t Intel8080Assembler::parseImmediate(std::string token) {
    if (token[0] == '\'') {
        if (token.size() == 3 && token[2] == '\'') {
            return static_cast<uint8_t>(token[1]);
        } else {
            error("invalid ascii literal");
        }
    }

    try {
        int base = 10;
        switch (token.back()) {
        case 'H':
            base = 16;
            break;
        case 'O':
            base = 8;
            break;
        case 'B':
            base = 2;
            break;
        }
        int imm = std::stoi(token, nullptr, base);
        if (imm < 0 || imm > 255) {
            throw std::out_of_range("");
        }
        return static_cast<uint8_t>(imm);
    } catch (std::invalid_argument &e) {
        error("could not parse immediate value");
    } catch (std::out_of_range &e) {
        error("immedate value is out of range (0-255)");
    }
    return 0;
}

uint8_t Intel8080Assembler::parseRegister(std::string token) {
    static const std::unordered_map<std::string, uint8_t> registers = {
        {"a", 7}, {"b", 0}, {"c", 1}, {"d", 2},
        {"e", 3}, {"f", 4}, {"l", 5}, {"m", 6}};

    auto it = registers.find(token);
    if (it == registers.end()) {
        error("expected register operand, got '", token, "'");
    }
    return it->second;
}

uint8_t Intel8080Assembler::parseRegisterPair(std::string token) {
    static const std::unordered_map<std::string, uint8_t> registers = {
        {"bc", 0}, {"de", 1}, {"hl", 2}, {"sp", 3}};

    auto it = registers.find(token);
    if (it == registers.end()) {
        error("expected register operand, got '", token, "'");
    }
    return it->second;
}

void Intel8080Assembler::emitByte(uint8_t byte) { memory[pc++] = byte; }

void Intel8080Assembler::emitWord(uint16_t word) {
    memory[pc++] = word & 0xff;
    memory[pc++] = (word >> 8) & 0xff;
}

template <typename... Ts> void Intel8080Assembler::error(Ts... args) {
    had_errors = true;
    std::ostringstream os;
    os << line_number << ": error: ";
    int unpack[]{0, ((os << args), 0)...};
    static_cast<void>(unpack);
    throw std::runtime_error(os.str());
}