
#include "infixParser.h"
#include <iostream>    
#include <string>      
#include <stdexcept>

// Most of the functionality is the same of mParser

InfixParser::InfixParser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentTokenIndex(0), root(nullptr), unmatchedParentheses(0) {}



Token& InfixParser::currentToken() {
    return tokens[currentTokenIndex];
}



// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding node to the AST.
// If the token type is invalid, it outputs an error message.
Node* InfixParser::expression(std::ostream& os) {
    Node* node = nullptr;
    try {
        node = assignmentExpression(os);
    } catch (...) {
        clearTree(node);
        throw;
    }
    return node;  
}


Node* InfixParser::assignmentExpression(std::ostream& os) {
    Node* node = logicalOrExpression(os);
    Node* valueNode = nullptr;

    try {
        Token op = currentToken();
        if (op.type == TokenType::ASSIGN) {
            if (node->type != NodeType::IDENTIFIER) {
                clearTree(node);
                throw std::runtime_error("Unexpected token at line " + std::to_string(op.line) + " column " + std::to_string(op.column) + ": " + op.value + "\n");
            }
            currentTokenIndex++;
            valueNode = assignmentExpression(os);

            Node* assignNode = new Node(NodeType::ASSIGN);
            assignNode->children.push_back(node);
            assignNode->children.push_back(valueNode);

            node = assignNode;
            valueNode = nullptr;  
        }
    } catch (...) {
        clearTree(valueNode); 
        clearTree(node);  
        throw; 
    }

    return node;
}



Node* InfixParser::logicalOrExpression(std::ostream& os) {
    Node* node = logicalXorExpression(os);
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_OR) {
            currentTokenIndex++;
            right = logicalXorExpression(os);

            Node* newNode = new Node(NodeType::LOGICAL_OR);
            newNode->children.push_back(node);
            newNode->children.push_back(right);

            node = newNode;
            right = nullptr;
        }
    } catch (...) {
        clearTree(right);
        clearTree(node);
        throw;
    }

    return node;
}

Node* InfixParser::logicalXorExpression(std::ostream& os) {
    Node* node = logicalAndExpression(os); 
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_XOR) {
            currentTokenIndex++;
            right = logicalAndExpression(os);

            Node* newNode = new Node(NodeType::LOGICAL_XOR);
            newNode->children.push_back(node);
            newNode->children.push_back(right);

            node = newNode;
            right = nullptr;
        }
    } catch (...) {
        clearTree(right); 
        clearTree(node);  
        throw; 
    }

    return node;
}



Node* InfixParser::logicalAndExpression(std::ostream& os) {
    Node* node = equalityExpression(os);  
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_AND) {
            Token op = currentToken();
            currentTokenIndex++;
            right = equalityExpression(os); 

            Node* andNode = new Node(NodeType::LOGICAL_AND);
            andNode->children.push_back(node);
            andNode->children.push_back(right);

            node = andNode; 
            right = nullptr; 
        }
    } catch (...) {
        clearTree(right); 
        clearTree(node);  
        throw; 
    }

    return node;
}


Node* InfixParser::equalityExpression(std::ostream& os) {
    Node* node = relationalExpression(os); 
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::EQUAL || currentToken().type == TokenType::NOT_EQUAL) {
            Token op = currentToken();
            currentTokenIndex++; 

            right = relationalExpression(os);

            Node* equalityNode = new Node(op.type == TokenType::EQUAL ? NodeType::EQUAL : NodeType::NOT_EQUAL);
            equalityNode->children.push_back(node);
            equalityNode->children.push_back(right);

            node = equalityNode; 
            right = nullptr; 
        }
    } catch (...) {
        clearTree(right); 
        clearTree(node);  
        throw; 
    }

    return node;
}


Node* InfixParser::relationalExpression(std::ostream& os) {
    Node* node = additiveExpression(os); 
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LESS || currentToken().type == TokenType::LESS_EQUAL ||
               currentToken().type == TokenType::GREATER || currentToken().type == TokenType::GREATER_EQUAL) {
            Token op = currentToken();
            currentTokenIndex++; 

            right = additiveExpression(os);

            Node* relationalNode = new Node(
                op.type == TokenType::LESS ? NodeType::LESS_THAN :
                op.type == TokenType::LESS_EQUAL ? NodeType::LESS_EQUAL :
                op.type == TokenType::GREATER ? NodeType::GREATER_THAN :
                NodeType::GREATER_EQUAL);
            relationalNode->children.push_back(node);
            relationalNode->children.push_back(right);

            node = relationalNode;
            right = nullptr; 
        }
    } catch (...) {
        clearTree(right); 
        clearTree(node); 
        throw; 
    }

    return node;
}


Node* InfixParser::additiveExpression(std::ostream& os) {
    Node* node = multiplicativeExpression(os);
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
            Token op = currentToken();
            currentTokenIndex++; 

            right = multiplicativeExpression(os); 

            Node* additiveNode = new Node(op.type == TokenType::ADD ? NodeType::ADD : NodeType::SUBTRACT);
            additiveNode->children.push_back(node);
            additiveNode->children.push_back(right);

            node = additiveNode; 
            right = nullptr; 
        }
    } catch (...) {
        clearTree(right); 
        clearTree(node);
        throw; 
    }

    return node;
}


Node* InfixParser::multiplicativeExpression(std::ostream& os) {
    Node* node = factor(os); 
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE || currentToken().type == TokenType::MODULO) {
            Token op = currentToken();
            currentTokenIndex++; 
            right = factor(os); 

            NodeType newNodeType; 
            if (op.type == TokenType::MULTIPLY) {
                newNodeType = NodeType::MULTIPLY;
            } else if (op.type == TokenType::DIVIDE) {
                newNodeType = NodeType::DIVIDE;
            } else { 
                newNodeType = NodeType::MODULO;
            }

            Node* newNode = new Node(newNodeType);
            newNode->children.push_back(node);
            newNode->children.push_back(right);

            node = newNode; 
            right = nullptr; 
        }
    } catch (...) {
        clearTree(right);
        clearTree(node); 
        throw; 
    }

    return node;
}

Node* InfixParser::factor(std::ostream& os) {
    Token& token = currentToken();
    Node* node = nullptr;  

    try {
        if (token.type == TokenType::NUMBER) {
            node = new Node(NodeType::NUMBER, std::stod(token.value));
            currentTokenIndex++;
        } 
        else if (token.type == TokenType::IDENTIFIER) {
            node = new Node(NodeType::IDENTIFIER, 0, token.value);
            currentTokenIndex++;
        } 
        else if (token.type == TokenType::LEFT_PAREN) {
            unmatchedParentheses++;
            currentTokenIndex++;
            node = expression(os);
            if (currentToken().type != TokenType::RIGHT_PAREN) {
                clearTree(node); 
                throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
            }
            unmatchedParentheses--;
            currentTokenIndex++;
        }
        else if (token.type == TokenType::BOOLEAN_TRUE) {
            node = new Node(NodeType::BOOLEAN_LITERAL, 1);
            currentTokenIndex++;
        }
        else if (token.type == TokenType::BOOLEAN_FALSE) {
            node = new Node(NodeType::BOOLEAN_LITERAL, 0);
            currentTokenIndex++;
        }
        else {
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
    } catch (...) {
        clearTree(node);
        throw;  
    }
    return node;
}


// This function initiates the parsing process and returns the root of the AST.
Node* InfixParser::parse(std::ostream& os) {
    try {
        root = expression(os);
        if (unmatchedParentheses != 0) {
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
        if (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
        if (currentToken().type != TokenType::END || currentToken().value != "END") {
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
    } catch (const std::runtime_error& e) {
        throw;  
    }
    return root;
}


// This function recursively deallocates memory used by the nodes in the AST, ensuring no memory leaks.
void InfixParser::clearTree(Node*& node) { 
    if (!node) return;
    for (Node*& child : node->children) {
        clearTree(child);
    }
    delete node;
    node = nullptr; 
}


// Destructor for the Parser class. It ensures that the memory used by the AST is deallocated.
InfixParser::~InfixParser() {
    clearTree(root);
}