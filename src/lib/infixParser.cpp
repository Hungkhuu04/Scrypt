// Include the header file "parse.h" which likely contains the declarations for the Parser class and its methods.
#include "infixParser.h"
#include <iostream>    
#include <string>      
#include <stdexcept>

// Constructor for the Parser class. Initializes the tokens vector and sets the current token index to 0.
InfixParser::InfixParser(const std::vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

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
            throw std::runtime_error("Error: Assignment must be to an identifier."); // <-- Throw exception
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

    //Number tokens
    if (token.type == TokenType::NUMBER) {
        Node* node = new Node(NodeType::NUMBER, std::stod(token.value));
        currentTokenIndex++; //consume or move to next token
        return node;
    // variable tokens
    } else if (token.type == TokenType::IDENTIFIER) {
        Node* idNode = new Node(NodeType::IDENTIFIER, 0, token.value);
        currentTokenIndex++; //consume or move to next token
        return idNode;
    //parenthesis stuff
    } else if (token.type == TokenType::LEFT_PAREN) {
        currentTokenIndex++; //consume open parenthesis
        Node* node = expression(os); //parse expression inside

        //check if there's a right parenthesis
        if (currentToken().type != TokenType::RIGHT_PAREN){
            throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column));
        }
        currentTokenIndex++; // Consume the right parenthesis.
        return node;
    }

    else if (token.type == TokenType::SUBTRACT) {
        currentTokenIndex++;
        Node* node = new Node(NodeType::SUBTRACT);
        node->children.push_back(factor(os));
        return node;
    }

    else {
        throw std::runtime_error("Unexpected token at line " + std::to_string(token.line) + " column " + std::to_string(token.column)); // <-- Throw exception
    }
    return nullptr;
}

Node* InfixParser::term(std::ostream& os) {
    Node* node = factor(os);

    while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE) {
        Token& token = currentToken();
        NodeType nodeType;

        //determine multiply or divide)
        if (token.type == TokenType::MULTIPLY) {
            nodeType = NodeType::MULTIPLY;
        } else {
            nodeType = NodeType::DIVIDE;
        }

        currentTokenIndex++;

        Node* newNode = new Node(nodeType);

        newNode->children.push_back(node); // first operand

        newNode->children.push_back(factor(os)); // second operand

        node = newNode;
    }
    
    return node;
}


// This function initiates the parsing process and returns the root of the AST.
Node* InfixParser::parse(std::ostream& os) {
    root = expression(os);
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column)); // <-- Throw exception
    }
    return root;
}

// This function recursively deallocates memory used by the nodes in the AST, ensuring no memory leaks.
void InfixParser::clearTree(Node* node) {
    if (!node) return;
    for (Node* child : node->children) {
        clearTree(child);
    }
    delete node;
}

std::vector<Node*> InfixParser::parseMultiple(std::ostream& os) {
    std::vector<Node*> roots;
    while (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        Node* root = parse(os);
        roots.push_back(root);
    }
    return roots;
}

// Destructor for the Parser class. It ensures that the memory used by the AST is deallocated.
InfixParser::~InfixParser() {
    clearTree(root);
}