// Include the header file "parse.h" which likely contains the declarations for the Parser class and its methods.
#include "parse.h"

// Include necessary standard libraries.
#include <iostream>    // Provides functionality for standard input/output.
#include <string>      // Provides the string data type and related functions.
#include <iostream>    // (Duplicate inclusion, this can be removed.)

// Constructor for the Parser class. Initializes the tokens vector and sets the current token index to 0.
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

// Returns the current token being processed.
Token& Parser::currentToken() {
    return tokens[currentTokenIndex];
}

// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding node to the AST.
// If the token type is invalid, it outputs an error message.
Node* Parser::expression(std::ostream& os) {
    if (currentToken().type == TokenType::LEFT_PAREN) {
        currentTokenIndex++;
        Node* node = nullptr;
        // Depending on the operator type, create a new node for the AST.
        switch (currentToken().type) {
            case TokenType::ADD:
                node = new Node(NodeType::ADD);
                break;
            case TokenType::SUBTRACT:
                node = new Node(NodeType::SUBTRACT);
                break;
            case TokenType::MULTIPLY:
                node = new Node(NodeType::MULTIPLY);
                break;
            case TokenType::DIVIDE:
                node = new Node(NodeType::DIVIDE);
                break;
            default:
                os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                exit(2);
        }
        currentTokenIndex++;
        // Check for valid tokens after the operator.
        if (currentToken().type != TokenType::NUMBER && currentToken().type != TokenType::LEFT_PAREN) {
            os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
            exit(2);
        }
        // Recursively add child nodes to the current node until a right parenthesis is found.
        while (currentToken().type != TokenType::RIGHT_PAREN) {
            node->children.push_back(expression(os));
        }
        currentTokenIndex++;
        return node;
    } else {
        return number(os);
    }
}

// This function parses a number token and returns a new Node object representing that number.
Node* Parser::number(std::ostream& os) {
    if (currentToken().type == TokenType::NUMBER) {
        Node* node = new Node(NodeType::NUMBER, std::stod(currentToken().value));
        currentTokenIndex++;
        return node;
    } else {
        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
        return 0;
    }
}

// This function initiates the parsing process and returns the root of the AST.
Node* Parser::parse(std::ostream& os) {
    root = expression(os);
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
    }
    return root;
}

// This function recursively deallocates memory used by the nodes in the AST, ensuring no memory leaks.
void Parser::clearTree(Node* node) {
    if (!node) return;
    for (Node* child : node->children) {
        clearTree(child);
    }
    delete node;
}

// Destructor for the Parser class. It ensures that the memory used by the AST is deallocated.
Parser::~Parser() {
    clearTree(root);
}
