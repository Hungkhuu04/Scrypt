#ifndef TOKEN_H
#define TOKEN_H
#include <string>

// Enum to represent different types of tokens

enum class TokenType {
    LEFT_PAREN, RIGHT_PAREN,
    ADD, SUBTRACT, MULTIPLY, DIVIDE,
    NUMBER,
    UNKNOWN
};

// Struct to represent a token
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

#endif