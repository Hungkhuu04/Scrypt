#include "lib/lex.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>

Lexer::Lexer(const std::string& input) : inputStream(input), line(1), col(1) {}

char Lexer::consume() {
    char current = inputStream.get();
    if (current == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return current;
}

bool Lexer::isDigit(char c) {
    return std::isdigit(c) || c == '.';
}

bool Lexer::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

Token Lexer::number() {
    int startCol = col;
    std::string num;
    bool hasDecimal = false;

    while (isDigit(inputStream.peek())) {
        char c = consume();
        if (c == '.') {
            if (hasDecimal) {
                return {TokenType::UNKNOWN, num + c, line, col - 1};
            }
            hasDecimal = true;

            if (!std::isdigit(inputStream.peek())) {
                return {TokenType::UNKNOWN, num + c, line, col};
            }
        }
        num += c;
    }

    if (num.front() == '.' || num.back() == '.') {
        return {TokenType::UNKNOWN, num, line, startCol};
    }

    return {TokenType::NUMBER, num, line, startCol};
}

Token Lexer::op() {
    int startCol = col;
    char op = consume();
    switch (op) {
        case '+': return {TokenType::ADD, "+", line, startCol};
        case '-': return {TokenType::SUBTRACT, "-", line, startCol};
        case '*': return {TokenType::MULTIPLY, "*", line, startCol};
        case '/': return {TokenType::DIVIDE, "/", line, startCol};
        default: return {TokenType::UNKNOWN, std::string(1, op), line, startCol};
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (inputStream.peek() != EOF) {
        char c = inputStream.peek();
        if (std::isspace(c)) {
            consume();
        } else if (c == '(') {
            tokens.push_back({TokenType::LEFT_PAREN, "(", line, col});
            consume();
        } else if (c == ')') {
            tokens.push_back({TokenType::RIGHT_PAREN, ")", line, col});
            consume();
        } else if (isDigit(c)) {
            Token numToken = number();
            if (numToken.type == TokenType::UNKNOWN) {
                tokens.push_back(numToken);
                return tokens;
            }
            tokens.push_back(numToken);
        } else if (isOperator(c)) {
            tokens.push_back(op());
        } else {
            tokens.push_back({TokenType::UNKNOWN, std::string(1, c), line, col});
            consume();
        }
    }
    tokens.push_back({TokenType::UNKNOWN, "END", line, col});
    return tokens;
}




int main() {
    // std::cout << "Enter your expression (Ctrl-D to end input):" << std::endl;

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
    }

    for (const auto& token : tokens) {
        std::cout << std::right << std::setw(4) << token.line << std::setw(5) << token.column << std::setw(2) << "  " << token.value << std::endl;
    }

    return 0;
}