
#ifndef TOKEN_H
#define TOKEN_H
#include <string>

// Token Header File for Lexer

// Enumerated Type to represent each type of tokens

enum class TokenType {
    IF, WHILE, PRINT, ELSE,
    LEFT_BRACE, RIGHT_BRACE,// curly braces { , }
    LEFT_PAREN, RIGHT_PAREN,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO,
    NUMBER,
    TRUE, FALSE, 
    IDENTIFIER,
    ASSIGN,
    UNKNOWN,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,
    EQUAL, NOT_EQUAL,
    LOGICAL_AND,
    LOGICAL_XOR,
    LOGICAL_OR,
    BOOLEAN_TRUE,
    BOOLEAN_FALSE,

};


// Struct to represent a token
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

#endif