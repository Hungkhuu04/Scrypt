#include "lib/lex.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>

int main() {
    std::string input;
    char ch;
    while (std::cin.get(ch)) {
        input += ch;
    }

    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        if (token.type == TokenType::UNKNOWN && token.value != "END") {
            std::cout << "Syntax error on line " << token.line << " column " << token.column << "." << std::endl;
            return 1;
        }
    }

    for (const auto& token : tokens) {
        std::cout << std::right << std::setw(4) << token.line << std::setw(5) << token.column << std::setw(2) << "  " << token.value << std::endl;
    }

    return 0;
}
