#include <iostream>
#include <iomanip>
#include "Lexer.h"

int main() {
    std::cout << "Enter your expression (Ctrl-D to end input):" << std::endl;

    std::string input;
    std::string line;
    while (std::getline(std::cin, line)) {
        input += line + "\n";
    }

    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        if (token.type == TokenType::UNKNOWN && token.value != "END") {
            std::cout << "Syntax error on line " << token.line << " column " << token.column << "." << std::endl;
            return 1;
        }
        std::cout << std::right << std::setw(4) << token.line << std::setw(5) << token.column << std::setw(3) << "  " << token.value << std::endl;
    }

    return 0;
}
