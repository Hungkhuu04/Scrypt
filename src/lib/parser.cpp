#include "parse.h"
#include <iostream>
#include<string>


Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

Token& Parser::currentToken() {
    return tokens[currentTokenIndex];
}

Node* Parser::expression() {
    if (currentToken().type == TokenType::LEFT_PAREN) {
        currentTokenIndex++;

        Node* node;
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
            /*default:
                std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                exit(2);*/
        }

        currentTokenIndex++;

        while (currentToken().type != TokenType::RIGHT_PAREN) {
            node->children.push_back(expression());
        }

        currentTokenIndex++;
        return node;
    } else {
        return number();
    }
}

Node* Parser::number() {
    if (currentToken().type == TokenType::NUMBER) {
        Node* node = new Node(NodeType::NUMBER, std::stod(currentToken().value));
        currentTokenIndex++;
        return node;
    } else {
        std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
        return 0;
    }
}

Node* Parser::parse() {
    Node* root = expression();
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        /*std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);*/
        return 0;
    }
    return root;
}

void Parser::clearTree(Node* node) {
    if (!node) return;

    for (Node* child : node->children) {
        clearTree(child);
    }

    delete node;
}

Parser::~Parser() {
    clearTree(parse());
}
