#include "parse.h"
#include <iostream>
#include<string>
#include<iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

Token& Parser::currentToken() {
    return tokens[currentTokenIndex];
}

Node* Parser::expression(std::ostream& os) {
    if (currentToken().type == TokenType::LEFT_PAREN) {
        currentTokenIndex++;
        Node* node = nullptr;
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
                node = new Node(NodeType::ASSIGN);
                break;
            default:
                os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                exit(2);
        }
        currentTokenIndex++;
        Node* firstChild = expression(os);
        if (node->type == NodeType::ASSIGN && firstChild->type != NodeType::IDENTIFIER) {
            os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
            exit(2);
        }
        node->children.push_back(firstChild);
        while (currentToken().type != TokenType::RIGHT_PAREN) {
            node->children.push_back(expression(os));
        }
        currentTokenIndex++;
        return node;
    } else if (currentToken().type == TokenType::IDENTIFIER) {
        Node* node = new Node(NodeType::IDENTIFIER, 0, currentToken().value);
        currentTokenIndex++;
        if (currentToken().type != TokenType::RIGHT_PAREN) {
            os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
            exit(2);
        }
        currentTokenIndex++;
        return node;
    } else {
        return number(os);
    }
}

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

Node* Parser::parse(std::ostream& os) {
    root = expression(os);
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        os << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
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
    clearTree(root);
}

std::vector<Node*> Parser::parseMultiple(std::ostream& os) {
    std::vector<Node*> roots;
    while (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        Node* root = parse(os);
        roots.push_back(root);
    }
    return roots;
}