#include "Lexer.h"
#include <cctype>
#include <iostream>
#include <iomanip>

Lexer::Lexer(const std::string& input) : input(input), length(input.size()), pos(0), line(1), col(1) {}

char Lexer::peek() {
    return pos < length ? input[pos] : '\0';
}

char Lexer::consume() {
    char current = input[pos];
    pos++;
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
    while (isDigit(peek())) {
        num += consume();
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
    while (pos < length) {
        char c = peek();
        if (std::isspace(c)) {
            consume();
        } else if (c == '(') {
            tokens.push_back({TokenType::LEFT_PAREN, "(", line, col});
            consume();
        } else if (c == ')') {
            tokens.push_back({TokenType::RIGHT_PAREN, ")", line, col});
            consume();
        } else if (isDigit(c)) {
            tokens.push_back(number());
        } else if (isOperator(c)) {
            tokens.push_back(op());
        } else {
            tokens.push_back({TokenType::UNKNOWN, std::string(1, c), line, col});
            break;
        }
    }
    tokens.push_back({TokenType::UNKNOWN, "END", line, col});
    return tokens;
}