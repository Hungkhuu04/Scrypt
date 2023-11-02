// Include the header file "parse.h" which likely contains the declarations for the Parser class and its methods.
#include "infixParser.h"
#include <iostream>    
#include <string>      
#include <stdexcept>

// Constructor for the Parser class. Initializes the tokens vector and sets the current token index to 0.
InfixParser::InfixParser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentTokenIndex(0), root(nullptr), unmatchedParentheses(0) {}


// Returns the current token being processed.
Token& InfixParser::currentToken() {
    return tokens[currentTokenIndex];
}


// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding node to the AST.
// If the token type is invalid, it outputs an error message.
Node* InfixParser::expression(std::ostream& os) {
    Node* node = term(os); // get first term
    
    while (true) { // Changed to while(true) loop for better control
        Token op = currentToken(); // Store operator token
        
        if (op.type == TokenType::ADD || op.type == TokenType::SUBTRACT || 
            op.type == TokenType::LESS || op.type == TokenType::LESS_EQUAL ||
            op.type == TokenType::GREATER || op.type == TokenType::GREATER_EQUAL ||
            op.type == TokenType::EQUAL || op.type == TokenType::NOT_EQUAL ||
            op.type == TokenType::LOGICAL_AND || op.type == TokenType::LOGICAL_OR ||
            op.type == TokenType::LOGICAL_XOR) {

            currentTokenIndex++;
            Node* right = term(os); // Get next term
            
            Node* newNode;
            
            // Create a new node based on the operator and attach left and right operands
            switch (op.type) {
                case TokenType::ADD: newNode = new Node(NodeType::ADD); break;
                case TokenType::SUBTRACT: newNode = new Node(NodeType::SUBTRACT); break;
                case TokenType::LESS: newNode = new Node(NodeType::LESS_THAN); break;
                case TokenType::LESS_EQUAL: newNode = new Node(NodeType::LESS_EQUAL); break;
                case TokenType::GREATER: newNode = new Node(NodeType::GREATER_THAN); break;
                case TokenType::GREATER_EQUAL: newNode = new Node(NodeType::GREATER_EQUAL); break;
                case TokenType::EQUAL: newNode = new Node(NodeType::EQUAL); break;
                case TokenType::NOT_EQUAL: newNode = new Node(NodeType::NOT_EQUAL); break;
                case TokenType::LOGICAL_AND: newNode = new Node(NodeType::LOGICAL_AND); break;
                case TokenType::LOGICAL_OR: newNode = new Node(NodeType::LOGICAL_OR); break;
                case TokenType::LOGICAL_XOR: newNode = new Node(NodeType::LOGICAL_XOR); break;
                default: throw std::runtime_error("Unexpected operator");
            }
            
            newNode->children.push_back(node);
            newNode->children.push_back(right);
            
            node = newNode;  // Make the new node the base for the next iteration
            
        } else if (op.type == TokenType::ASSIGN) {
            if (node->type != NodeType::IDENTIFIER) {
                clearTree(root);  // Clear the memory before throwing
                throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
            }
            currentTokenIndex++; // Consume or move to the next token
            Node* valueNode = expression(os);
            Node* assignNode = new Node(NodeType::ASSIGN);
            assignNode->children.push_back(node);
            assignNode->children.push_back(valueNode);
            node = assignNode;
        } else {
            break;
        }
    }
    
    return node;
}

Node* InfixParser::factor(std::ostream& os) {
    Token& token = currentToken();
    // Number tokens
    if (token.type == TokenType::NUMBER) {
        Node* node = new Node(NodeType::NUMBER, std::stod(token.value));
        currentTokenIndex++;
        return node;
    } 
    // Variable tokens
    else if (token.type == TokenType::IDENTIFIER) {
        Node* idNode = new Node(NodeType::IDENTIFIER, 0, token.value);
        currentTokenIndex++;
        return idNode;
    } 
    // Parenthesis stuff
    else if (token.type == TokenType::LEFT_PAREN) {
        unmatchedParentheses++;
        currentTokenIndex++;
        Node* node = expression(os);

        // Only check for the right parenthesis here
        if (currentToken().type == TokenType::RIGHT_PAREN){
            unmatchedParentheses--;
            currentTokenIndex++;
        }
        return node;
    }
    else if (token.type == TokenType::BOOLEAN_TRUE) {
        Node* node = new Node(NodeType::BOOLEAN_LITERAL, 1); // 1 for true
        currentTokenIndex++;
        return node;
    }
    else if (token.type == TokenType::BOOLEAN_FALSE) {
        Node* node = new Node(NodeType::BOOLEAN_LITERAL, 0); // 0 for false
        currentTokenIndex++;
        return node;
    }
    else {
        clearTree(root);
        throw std::runtime_error("Unexpected token at line " + std::to_string(token.line) + " column " + std::to_string(token.column) + ": " + token.value + "\n");
    }
    return nullptr;
}

Node* InfixParser::term(std::ostream& os) {
    Node* node = factor(os);

    while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE) {
        Token& token = currentToken();
        NodeType nodeType;

        // determine multiply or divide
        if (token.type == TokenType::MULTIPLY) {
            nodeType = NodeType::MULTIPLY;
        } else {
            nodeType = NodeType::DIVIDE;
        }

        currentTokenIndex++;

        Node* newNode = new Node(nodeType);

        newNode->children.push_back(node); // first operand
        newNode->children.push_back(factor(os)); // second operand

        // Check if a valid right-hand operand was received
        if (newNode->children.back() == nullptr) {
            clearTree(root); 
            throw std::runtime_error("Unexpected token at line " + std::to_string(token.line) + " column " + std::to_string(token.column) + ": " + token.value + "\n");
        }

        node = newNode;
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
        if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
    } catch (const std::runtime_error& e) {
        clearTree(root);  // Clear the memory
        throw;  // Re-throw the caught exception
    }
    return root;
}

// This function recursively deallocates memory used by the nodes in the AST, ensuring no memory leaks.
void InfixParser::clearTree(Node*& node) {  // Changed node to a reference to a pointer
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