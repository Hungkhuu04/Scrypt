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
    Node* node = nullptr;
    try {
        node = logicalOrExpression(os);  // get first term

        while (true) {
            Token op = currentToken();  // Store operator token

            if (op.type == TokenType::ADD || op.type == TokenType::SUBTRACT || 
                op.type == TokenType::LESS || op.type == TokenType::LESS_EQUAL ||
                op.type == TokenType::GREATER || op.type == TokenType::GREATER_EQUAL ||
                op.type == TokenType::EQUAL || op.type == TokenType::NOT_EQUAL ||
                op.type == TokenType::LOGICAL_AND || op.type == TokenType::LOGICAL_OR ||
                op.type == TokenType::LOGICAL_XOR) {

                currentTokenIndex++;
                Node* right = term(os);  // Get next term

                Node* newNode = new Node(op.type == TokenType::ADD ? NodeType::ADD : NodeType::SUBTRACT);

                newNode->children.push_back(node);
                newNode->children.push_back(right);

                node = newNode;  // Make the new node the base for the next iteration
            } else if (op.type == TokenType::ASSIGN) {
                if (node->type != NodeType::IDENTIFIER) {
                    clearTree(node);
                    throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
                }
                currentTokenIndex++;
                Node* valueNode = expression(os);  // Recursively call expression

                Node* assignNode = new Node(NodeType::ASSIGN);
                assignNode->children.push_back(node);
                assignNode->children.push_back(valueNode);

                node = assignNode;
            } else {
                break;  // If none of the operators match, break out of the loop
            }
        }
    } catch (...) {
        clearTree(node);  // Clean up whatever was built up to this point
        throw;  // Re-throw the current exception
    }

    return node;  // Return the constructed node
}

Node* InfixParser::logicalOrExpression(std::ostream& os) {
    Node* node = logicalAndExpression(os); // Next level of precedence
    while (currentToken().type == TokenType::LOGICAL_OR) {
        currentTokenIndex++;
        Node* right = logicalAndExpression(os); // Get the next term
        Node* newNode = new Node(NodeType::LOGICAL_OR);
        newNode->children.push_back(node);
        newNode->children.push_back(right);
        node = newNode; // This node now becomes the left-hand operand for any further LOGICAL_ORs
    }
    return node;
}

Node* InfixParser::logicalAndExpression(std::ostream& os) {
    Node* node = equalityExpression(os);  // Start with a higher precedence level expression

    while (currentToken().type == TokenType::LOGICAL_AND) {
        Token op = currentToken();
        currentTokenIndex++; // Consume the '&&' token
        Node* right = equalityExpression(os); // Parse the right-hand operand
        
        // Now create a new logical AND node with the left and right operands
        Node* andNode = new Node(NodeType::LOGICAL_AND);
        andNode->children.push_back(node);
        andNode->children.push_back(right);

        node = andNode; // This node now becomes the left-hand operand for any further LOGICAL_ANDs
    }
    
    return node;
}

Node* InfixParser::equalityExpression(std::ostream& os) {
    Node* node = relationalExpression(os);  // Start with a higher precedence level expression

    // Continue parsing while the current token is an equality or inequality operator
    while (currentToken().type == TokenType::EQUAL || currentToken().type == TokenType::NOT_EQUAL) {
        Token op = currentToken();
        currentTokenIndex++; // Consume the equality/inequality token
        
        // Parse the right-hand operand using another call to relationalExpression()
        Node* right = relationalExpression(os);
        
        // Create a new node based on the operator
        Node* equalityNode = nullptr;
        if (op.type == TokenType::EQUAL) {
            equalityNode = new Node(NodeType::EQUAL);
        } else if (op.type == TokenType::NOT_EQUAL) {
            equalityNode = new Node(NodeType::NOT_EQUAL);
        } else {
            // This should not happen, but it's good practice to handle unexpected cases
            clearTree(node);  // Clear memory to prevent leaks
            throw std::runtime_error("Invalid operator type for an equality expression");
        }

        // Add left and right operands to the new node
        equalityNode->children.push_back(node);
        equalityNode->children.push_back(right);

        node = equalityNode; // This node now becomes the left-hand operand for any further equality operations
    }
    
    return node;
}

Node* InfixParser::relationalExpression(std::ostream& os) {
    Node* node = additiveExpression(os); // Start with a higher precedence level expression

    // Continue parsing while the current token is a relational operator
    while (currentToken().type == TokenType::LESS || currentToken().type == TokenType::LESS_EQUAL ||
           currentToken().type == TokenType::GREATER || currentToken().type == TokenType::GREATER_EQUAL) {
        Token op = currentToken();
        currentTokenIndex++; // Consume the relational token
        
        // Parse the right-hand operand using another call to additiveExpression()
        Node* right = additiveExpression(os);
        
        // Create a new node based on the operator
        Node* relationalNode = nullptr;
        switch (op.type) {
            case TokenType::LESS:
                relationalNode = new Node(NodeType::LESS_THAN);
                break;
            case TokenType::LESS_EQUAL:
                relationalNode = new Node(NodeType::LESS_EQUAL);
                break;
            case TokenType::GREATER:
                relationalNode = new Node(NodeType::GREATER_THAN);
                break;
            case TokenType::GREATER_EQUAL:
                relationalNode = new Node(NodeType::GREATER_EQUAL);
                break;
            default:
                // This should not happen, but it's good practice to handle unexpected cases
                clearTree(node); // Clear memory to prevent leaks
                throw std::runtime_error("Invalid operator type for a relational expression");
        }

        // Add left and right operands to the new node
        relationalNode->children.push_back(node);
        relationalNode->children.push_back(right);

        node = relationalNode; // This node now becomes the left-hand operand for any further relational operations
    }
    
    return node;
}

Node* InfixParser::additiveExpression(std::ostream& os) {
    Node* node = multiplicativeExpression(os); // Start with the highest precedence expressions

    while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
        Token op = currentToken();
        currentTokenIndex++; // Consume the operator token

        Node* right = multiplicativeExpression(os); // Parse the next multiplicative expression

        Node* additiveNode;
        switch (op.type) {
            case TokenType::ADD:
                additiveNode = new Node(NodeType::ADD);
                break;
            case TokenType::SUBTRACT:
                additiveNode = new Node(NodeType::SUBTRACT);
                break;
            default:
                // This shouldn't happen, but just in case
                clearTree(node);
                throw std::runtime_error("Invalid operator type for an additive expression");
        }

        // Add the left and right operands to the new node
        additiveNode->children.push_back(node);
        additiveNode->children.push_back(right);

        node = additiveNode; // Update node to the newly created one for subsequent loops
    }

    return node; // Return the root of the constructed subtree for additive expressions
}

Node* InfixParser::multiplicativeExpression(std::ostream& os) {
    Node* node = factor(os); // Get the first operand

    while (currentToken().type == TokenType::MULTIPLY || 
           currentToken().type == TokenType::DIVIDE || 
           currentToken().type == TokenType::LOGICAL_AND) { // Added TokenType::POWER handling

        Token op = currentToken();
        currentTokenIndex++; // Move past the operator token

        Node* right = factor(os); // Get the next factor (operand)

        Node* newNode = nullptr;
        // Create a new node based on the operator and attach left and right operands
        switch (op.type) {
            case TokenType::MULTIPLY:
                newNode = new Node(NodeType::MULTIPLY);
                break;
            case TokenType::DIVIDE:
                newNode = new Node(NodeType::DIVIDE);
                break;
            case TokenType::LOGICAL_AND: // Handling the power operator
                newNode = new Node(NodeType::LOGICAL_AND);
                break;
            default:
                // It's a good practice to handle unexpected cases,
                // even though the logic should not reach this point.
                clearTree(node);
                clearTree(right);
                throw std::runtime_error("Unexpected token in multiplicativeExpression");
        }

        newNode->children.push_back(node);
        newNode->children.push_back(right);

        node = newNode; // The new node becomes the current node for the next iteration
    }

    return node;
}

Node* InfixParser::factor(std::ostream& os) {
    Token& token = currentToken();
    Node* node = nullptr;  // Initialize node pointer to nullptr

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
                clearTree(node); // Clear the current sub-expression
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
        clearTree(node); // Clear up any memory allocated before re-throwing
        throw;  // Re-throw the exception to be handled further up the call stack
    }
    return node;
}

Node* InfixParser::term(std::ostream& os) {
    Node* node = factor(os);

    while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE) {
        Token& token = currentToken();
        NodeType nodeType = (token.type == TokenType::MULTIPLY) ? NodeType::MULTIPLY : NodeType::DIVIDE;

        Node* newNode = nullptr;
        try {
            newNode = new Node(nodeType);
            currentTokenIndex++;

            newNode->children.push_back(node);  // First operand

            Node* rightNode = factor(os);  // Attempt to get the second operand
            if (!rightNode) {
                clearTree(newNode); // Clear the new operation node
                throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
            }
            newNode->children.push_back(rightNode);

            node = newNode;  // Update the current node to the new operation node
        } catch (...) {
            clearTree(newNode);  // Clean up in case of exception
            throw;  // Re-throw the exception to be handled further up the call stack
        }
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