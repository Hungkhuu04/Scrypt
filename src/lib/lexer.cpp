
#include "lex.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

Lexer::Lexer(const string& input) : inputStream(input), line(1), col(1) {}

// Outputs the Error Code when there is an incorrect S expression

bool Lexer::isSyntaxError(std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        if (token.type == TokenType::UNKNOWN && token.value != "END") {
            std::cout << "Syntax error on line " << token.line << " column " << token.column << "." << std::endl;
            return true;
        }
    }
    return false;
}

//Reads the current character from the stream and keeps track of the column and line.
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

//Checks if the character is a valid Digit.
bool Lexer::isDigit(char c) {
    return isdigit(c) || c == '.';
}

void Lexer::increaseLine(int line_count) {
    for(int i = 0; i < line_count; i++ ){
        line++;
    }
}
//Checks if the character is a valid operator. 
bool Lexer::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

/*Handles the tokenization of a numerical value. Specifically, if the number 
follows the criteria for a valid number. It checks if the current character is a valid numeric value.
It checks the decimals to see what kinda of number it is - Integer or Float*/
Token Lexer::number() {
    int startCol = col;
    string num;
    bool hasDecimal = false;
    while (isDigit(inputStream.peek())) {
        char c = consume();
        if (c == '.') {
            if (hasDecimal) {
                return {TokenType::UNKNOWN, num + c, line, col - 1};
            }
            hasDecimal = true;
            if (!isdigit(inputStream.peek())) {
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

//Responsible for creating and tokenizing operators.
Token Lexer::op() {
    int startCol = col;
    char op = consume();
    switch (op) {
        case '+': return {TokenType::ADD, "+", line, startCol};
        case '-': return {TokenType::SUBTRACT, "-", line, startCol};
        case '*': return {TokenType::MULTIPLY, "*", line, startCol};
        case '/': return {TokenType::DIVIDE, "/", line, startCol};
        default: return {TokenType::UNKNOWN, string(1, op), line, startCol};
    }
}

/*Is responsible for tokenizing the input stream. Classifies the differnet tokens
and puts them in a vector.*/
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (inputStream.peek() != EOF) {
        char c = inputStream.peek();
        if (isspace(c)) {
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
        } else if (isalpha(c) || c == '_') {
            std::string identifier;
            int identifierStartCol = col;
            while (isalnum(inputStream.peek()) || inputStream.peek() == '_') {
                identifier += consume();
            }
            tokens.push_back({TokenType::IDENTIFIER, identifier, line, identifierStartCol});
        } else if (c == '=') {
            tokens.push_back({TokenType::ASSIGN, "=", line, col});
            consume();
        } else {
            tokens.push_back({TokenType::UNKNOWN, std::string(1, c), line, col});
            consume();
        }
        if (isSyntaxError(tokens)) {
        }
    }
    tokens.push_back({TokenType::UNKNOWN, "END", line, col});
    return tokens;
}
