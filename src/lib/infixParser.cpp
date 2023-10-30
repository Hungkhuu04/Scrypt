// Include the header file "parse.h" which likely contains the declarations for the Parser class and its methods.
#include "infixParser.h"
#include <iostream>    
#include <string>      
#include <stdexcept>

// Constructor for the Parser class. Initializes the tokens vector and sets the current token index to 0.
InfixParser::InfixParser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentTokenIndex(0), root(nullptr) {}

// Returns the current token being processed.
Token& InfixParser::currentToken() {
    return tokens[currentTokenIndex];
}


// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding node to the AST.
// If the token type is invalid, it outputs an error message.
Node* InfixParser::expression(std::ostream& os) {
    Node* node = term(os); //get first term
    while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
        Token op = currentToken(); // store operator token
        currentTokenIndex++;
        if (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
            clearTree(node);
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
        Node* right = term(os); // get next term
        
        // create a new node based on the operator and attach left and right operands.
        Node* newNode;
        if(op.type == TokenType::ADD) {
            newNode = new Node(NodeType::ADD);
        } else { // TokenType::SUBTRACT
            newNode = new Node(NodeType::SUBTRACT);
        }

        newNode->children.push_back(node);
        newNode->children.push_back(right);

        //make new node the base for the next iteration.
        node = newNode;
    }
    if (currentToken().type == TokenType::ASSIGN) {
        if (node->type != NodeType::IDENTIFIER) {
            clearTree(node);  // Clear the memory before throwing
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
        }
        currentTokenIndex++; //consume or move to next token
        Node* valueNode = expression(os);
        Node* assignNode = new Node(NodeType::ASSIGN);
        assignNode->children.push_back(node);
        assignNode->children.push_back(valueNode);
        node = assignNode;
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
    else {
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
            clearTree(node); 
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