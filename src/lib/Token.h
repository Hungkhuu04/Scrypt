#ifndef TOKEN_H
#define TOKEN_H
#include <string>

// Token Header File for Lexer

// Enumerated Type to represent each type of tokens

enum class TokenType {
    LEFT_PAREN, RIGHT_PAREN,
    ADD, SUBTRACT, MULTIPLY, DIVIDE,
    NUMBER,
    IDENTIFIER,  // New token type for identifiers (variables)
    ASSIGN,      // New token type for the assignment operator (=)
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