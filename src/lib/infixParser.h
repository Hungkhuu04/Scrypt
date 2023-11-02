
#ifndef INFIX_PARSER_H
#define INFIX_PARSER_H

#include "lex.h"
#include <vector>
#include <iostream>
#include<map>

// Enum representing different node types in the AST
enum class NodeType {
    ADD, SUBTRACT, MULTIPLY, DIVIDE, NUMBER, IDENTIFIER, ASSIGN,
    BOOLEAN, LESS_THAN, LESS_EQUAL, GREATER_THAN, GREATER_EQUAL,
    EQUAL, NOT_EQUAL, LOGICAL_AND, LOGICAL_XOR, LOGICAL_OR, 
    IF, WHILE, PRINT,MODULO,
    BOOLEAN_LITERAL
};

// Structure representing a node in the AST
struct Node {
    NodeType type;
    double value;
    bool boolValue;
    std::string identifier;
    std::vector<Node*> children;
    Node(NodeType t, double v = 0, const std::string& id = "") 
        : type(t), value(v), identifier(id) {}
};


class InfixParser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;
    Node* root;
    int unmatchedParentheses = 0;
    Node* logicalOrExpression(std::ostream& os);
    Node* logicalAndExpression(std::ostream& os);
    Node* equalityExpression(std::ostream& os);
    Node* relationalExpression(std::ostream& os);
    Node* additiveExpression(std::ostream& os); 
    Node* multiplicativeExpression(std::ostream& os);
    
    Token& currentToken();
    Node* expression(std::ostream& os = std::cerr); //Arithmetic expression like + or -
    Node* term(std::ostream& os = std::cerr); // For multiplication and division
    Node* factor(std::ostream& os = std::cerr); // The most basic element whether it is an expression in parenthesis or numbers or variables.

public:
    InfixParser(const std::vector<Token>& tokens);
    ~InfixParser();
    Node* parse(std::ostream& os = std::cerr);
    void clearTree(Node*& node);
};

#endif
