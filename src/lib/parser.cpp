#include "parse.h"
#include <iostream>
#include<string>
#include<iostream>

//Used for accessing current token. 

Parser::Parser(const std::vector<Token> &tokens, int lineCount) 
    : tokens(tokens), currentTokenIndex(0), currentLineNumber(lineCount) {}

Token &Parser::currentToken(){
    return tokens[currentTokenIndex];
}

/*Parses the expression and sets up the AST.
It is achieved by checking the current token type and adding that token to AST
If the token type is invalid it wont add it and then throw the error.
Everytime thers a new braket ( or ) it extends to the children nodes.
*/
Node *Parser::expression(std::ostream &os){
    if (currentToken().type == TokenType::LEFT_PAREN){
        currentTokenIndex++;
        Node *node = nullptr;
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
            case TokenType::ASSIGN:
                {
                    node = new Node(NodeType::ASSIGN);
                    currentTokenIndex++;
                    if (currentToken().type != TokenType::IDENTIFIER) {
                        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                        exit(2);
                    }
                    bool nonIdentifierEncountered = false;
                    while (currentToken().type != TokenType::RIGHT_PAREN) {
                        if (nonIdentifierEncountered && currentToken().type != TokenType::RIGHT_PAREN) {
                            os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                            exit(2);
                        }
                        node->children.push_back(expression(os));
                        if (node->children.back()->type != NodeType::IDENTIFIER) {
                            nonIdentifierEncountered = true;
                        }
                    }
                    if (node->children.size() <= 1) {
                        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                        exit(2);
                    }
                    currentTokenIndex++;
                }
                break;
            default:
                break;
        }
        currentTokenIndex++;
        while (currentToken().type != TokenType::RIGHT_PAREN){
            node->children.push_back(expression(os));
        }
        currentTokenIndex++;
        return node;
    } else if (currentToken().type == TokenType::IDENTIFIER){
        Node *node = new Node(NodeType::IDENTIFIER, 0, currentToken().value);
        currentTokenIndex++;
        return node;
    } else {
        return number(os);
    }
}

// Resposible for parsing the tokens and setting up the AST.
Node *Parser::parse(std::ostream &os){
    root = expression(os);
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END"){
        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
    }
    return root;
}

// Recursively deallocates memory being used by Nodes in the AST. Makes sure of no memory leaks.
void Parser::clearTree(Node *node){
    if (!node)
        return;
    for (Node *child : node->children){
        clearTree(child);
    }
    delete node;
}

// Desctructor
Parser::~Parser(){
    clearTree(root);
}

Node *Parser::number(std::ostream &os) {
    if (currentToken().type == TokenType::NUMBER) {
        Node *node = new Node(NodeType::NUMBER, std::stod(currentToken().value));
        currentTokenIndex++;
        return node;
    } else {
        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
    }
}