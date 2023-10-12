<<<<<<< Updated upstream
=======
#pragma once

#include <string>
#include <vector>
#include "Token.h"

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    char peek();
    char consume();
    bool isDigit(char c);
    bool isOperator(char c);
    Token number();
    Token op();

    std::string input;
    int length;
    int pos;
    int line;
    int col;
};
>>>>>>> Stashed changes
