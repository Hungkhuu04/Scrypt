#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "Token.h"

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    char consume();
    bool isDigit(char c);
    bool isOperator(char c);
    Token number();
    Token op();

    std::istringstream inputStream;
    int line;
    int col;
};
