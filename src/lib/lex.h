#ifndef LEX_H
#define LEX_H
#include <string>
#include <vector>
#include <sstream>
#include "Token.h"

// Lexer Header Definition

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    bool isSyntaxError(std::vector<Token>& tokens);
    char consume();
    bool isDigit(char c);
    bool isOperator(char c);
    Token number();
    Token op();
    std::istringstream inputStream;
    int line;
    int col;

};

#endif