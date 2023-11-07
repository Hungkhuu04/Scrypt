
#ifndef M_PARSER_H
#define M_PARSER_H

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
    BOOLEAN_LITERAL,
    ASSIGNMENT_STATEMENT,
    WHILE_STATEMENT,
    IF_STATEMENT,
    PRINT_STATEMENT,BLOCK,
    END, LEFT_BRACE, RIGHT_BRACE,
    NEWLINE,
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


class mParser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;
    Node* root;
    int unmatchedParentheses = 0;

    Node*assignmentExpression(std::ostream& os);
    Node* logicalOrExpression(std::ostream& os);
    Node* logicalAndExpression(std::ostream& os);
    Node* equalityExpression(std::ostream& os);
    Node* relationalExpression(std::ostream& os);
    Node* additiveExpression(std::ostream& os); 
    Node* multiplicativeExpression(std::ostream& os);
    Node* logicalXorExpression(std::ostream& os);
    Node* expression(std::ostream& os = std::cerr); //Arithmetic expression like + or -

    Token& currentToken();
    Node* factor(std::ostream& os = std::cerr); // The most basic element whether it is an expression in parenthesis or numbers or variables.

    Node* ifStatement(std::ostream& os);
    Node* whileStatement(std::ostream& os);
    Node* statement(std::ostream& os);
    Node* printStatement(std::ostream& os);
    Node* assignmentStatement(std::ostream& os);
    Node* parseBlock(std::ostream& os);

public:
    mParser(const std::vector<Token>& tokens);
    ~mParser();
    Node* parse(std::ostream& os);
    void clearTree(Node*& node);
};

#endif