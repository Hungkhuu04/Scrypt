
#ifndef M_PARSER_H
#define M_PARSER_H

#include "lex.h"
#include <vector>
#include <iostream>
#include<map>

// Enum representing different mnode types in the AST
enum class mNodeType {
    ADD, SUBTRACT, MULTIPLY, DIVIDE, NUMBER, IDENTIFIER, ASSIGN,
    BOOLEAN, LESS_THAN, LESS_EQUAL, GREATER_THAN, GREATER_EQUAL,
    EQUAL, NOT_EQUAL, LOGICAL_AND, LOGICAL_XOR, LOGICAL_OR, 
    IF, WHILE, PRINT,MODULO,
    BOOLEAN_LITERAL,
    ASSIGNMENT_STATEMENT,
    WHILE_STATEMENT,
    IF_STATEMENT,
    PRINT_STATEMENT,BLOCK,
    END, LEFT_BRACE, RIGHT_BRACE,
    NEWLINE,
};

// Structure representing a mnode in the AST
struct mNode {
    mNodeType type;
    double value;
    bool boolValue;
    std::string identifier;
    std::vector<mNode*> children;
    mNode(mNodeType t, double v = 0, const std::string& id = "") 
        : type(t), value(v), identifier(id) {}
};


class mParser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;
    mNode* root;
    int unmatchedParentheses = 0;

    mNode*assignmentExpression(std::ostream& os);
    mNode* logicalOrExpression(std::ostream& os);
    mNode* logicalAndExpression(std::ostream& os);
    mNode* equalityExpression(std::ostream& os);
    mNode* relationalExpression(std::ostream& os);
    mNode* additiveExpression(std::ostream& os); 
    mNode* multiplicativeExpression(std::ostream& os);
    mNode* logicalXorExpression(std::ostream& os);
    mNode* expression(std::ostream& os = std::cerr); //Arithmetic expression like + or -

    Token& currentToken();
    mNode* factor(std::ostream& os = std::cerr); // The most basic element whether it is an expression in parenthesis or numbers or variables.

    mNode* ifStatement(std::ostream& os);
    mNode* whileStatement(std::ostream& os);
    mNode* statement(std::ostream& os);
    mNode* printStatement(std::ostream& os);
    mNode* assignmentStatement(std::ostream& os);
    mNode* parseBlock(std::ostream& os);

public:
    mParser(const std::vector<Token>& tokens);
    ~mParser();
    mNode* parse(std::ostream& os);
    void clearTree(mNode*& mnode);
};

#endif
