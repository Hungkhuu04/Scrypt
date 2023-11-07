
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
    PRINT_STATEMENT,
    ELSE_STATEMENT,
    BLOCK, 
    END, LEFT_BRACE, RIGHT_BRACE,
    NEWLINE,
};

// Structure representing a mnode in the AST
struct mNode {
    mNodeType type;
    double value;
    bool boolValue;
    std::string identifier;
    
    mNode* condition;        // Used for conditional (if) and loop (while) nodes
    mNode* thenBranch;       // Used for the 'then' part of an if statement
    mNode* elseBranch;       // Used for the 'else' part of an if statement
    mNode* body;
    mNode* Lvalue;             // Used for the body of loops (while) and blocks
    std::vector<mNode*> children; // Used for expressions and storing multiple statements in a block
    std::vector<mNode*> statements;

    // Constructors for different types of nodes
    mNode(mNodeType t, double v = 0, bool bv = false, const std::string& id = "")
        : type(t), value(v), boolValue(bv), identifier(id), condition(nullptr),
          thenBranch(nullptr), elseBranch(nullptr), body(nullptr) {}

    // Destructor to deallocate dynamically allocated nodes
    ~mNode() {
        delete condition;
        delete thenBranch;
        delete elseBranch;
        delete body;
        for (auto child : children) {
            delete child;
        }
    }

    // Prevent copy semantics to avoid double deletion
    mNode(const mNode& other) = delete;
    mNode& operator=(const mNode& other) = delete;

    // Allow move semantics for ownership transfer
    mNode(mNode&& other) noexcept
        : type(other.type), value(other.value), boolValue(other.boolValue),
          identifier(std::move(other.identifier)), condition(other.condition),
          thenBranch(other.thenBranch), elseBranch(other.elseBranch), body(other.body),
          children(std::move(other.children)) {
        
        other.condition = nullptr;
        other.thenBranch = nullptr;
        other.elseBranch = nullptr;
        other.body = nullptr;
    }

    mNode& operator=(mNode&& other) noexcept {
        if (this != &other) {
            this->~mNode(); // Clean up current resources
            
            type = other.type;
            value = other.value;
            boolValue = other.boolValue;
            identifier = std::move(other.identifier);
            condition = other.condition;
            thenBranch = other.thenBranch;
            elseBranch = other.elseBranch;
            body = other.body;
            children = std::move(other.children);

            other.condition = nullptr;
            other.thenBranch = nullptr;
            other.elseBranch = nullptr;
            other.body = nullptr;
        }
        return *this;
    }
};

class mParser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;
    mNode* root;
    int unmatchedParentheses = 0;

    // All these expression functions serve to parse a singular expression.
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
    mNode* parseIfStatement(std::ostream& os);
    mNode* parseWhileStatement(std::ostream& os);
    mNode* parsePrintStatement(std::ostream& os);
    mNode* parseBlock(std::ostream& os);
    mNode* parseStatement(std::ostream& os);

    void advance();

public:
    mParser(const std::vector<Token>& tokens);
    ~mParser();
    mNode* parse(std::ostream& os);
    void clearTree(mNode*& mnode);
};

#endif
