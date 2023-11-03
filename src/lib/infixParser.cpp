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

void InfixParser::consume(TokenType expected, std::ostream& os) {
    if (currentToken().type == expected) {
        // Consume the token and move to the next
        currentTokenIndex++;
    } else {
        throw std::runtime_error("Not expecting this token");
    }
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
            valueNode = assignmentExpression(os);  // Handle right-associative behavior

            Node* assignNode = new Node(NodeType::ASSIGN);
            assignNode->children.push_back(node);
            assignNode->children.push_back(valueNode);

            node = assignNode;
            valueNode = nullptr;  
        }
    } catch (...) {
        clearTree(valueNode); // valueNode might have been partially constructed, so clean it up.
        clearTree(node);  // Clean up the entire node tree
        throw; // Re-throw the current exception
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
    Node* node = logicalAndExpression(os); // Start with a lower precedence expression
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
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(node);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return node;
}



Node* InfixParser::logicalAndExpression(std::ostream& os) {
    Node* node = equalityExpression(os);  // Start with a lower precedence level expression
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_AND) {
            Token op = currentToken();
            currentTokenIndex++; // Consume the '&&' token
            right = equalityExpression(os); // Parse the right-hand operand

            Node* andNode = new Node(NodeType::LOGICAL_AND);
            andNode->children.push_back(node);
            andNode->children.push_back(right);

            node = andNode; // This node now becomes the left-hand operand for any further LOGICAL_ANDs
            right = nullptr; // The right node is now managed by andNode, clear the pointer without deleting
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(node);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return node;
}


Node* InfixParser::equalityExpression(std::ostream& os) {
    Node* node = relationalExpression(os);  // Start with a higher precedence level expression
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::EQUAL || currentToken().type == TokenType::NOT_EQUAL) {
            Token op = currentToken();
            currentTokenIndex++; // Consume the equality/inequality token

            right = relationalExpression(os);

            Node* equalityNode = new Node(op.type == TokenType::EQUAL ? NodeType::EQUAL : NodeType::NOT_EQUAL);
            equalityNode->children.push_back(node);
            equalityNode->children.push_back(right);

            node = equalityNode; // This node now becomes the left-hand operand for any further equality operations
            right = nullptr; // Prevents deleting right in case of an exception
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(node);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return node;
}


Node* InfixParser::relationalExpression(std::ostream& os) {
    Node* node = additiveExpression(os); // Start with a higher precedence level expression
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::LESS || currentToken().type == TokenType::LESS_EQUAL ||
               currentToken().type == TokenType::GREATER || currentToken().type == TokenType::GREATER_EQUAL) {
            Token op = currentToken();
            currentTokenIndex++; // Consume the relational token

            right = additiveExpression(os);

            Node* relationalNode = new Node(
                op.type == TokenType::LESS ? NodeType::LESS_THAN :
                op.type == TokenType::LESS_EQUAL ? NodeType::LESS_EQUAL :
                op.type == TokenType::GREATER ? NodeType::GREATER_THAN :
                NodeType::GREATER_EQUAL); // The last condition must be GREATER_EQUAL
            relationalNode->children.push_back(node);
            relationalNode->children.push_back(right);

            node = relationalNode; // This node now becomes the left-hand operand for any further relational operations
            right = nullptr; // Prevents deleting right in case of an exception
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(node);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return node;
}


Node* InfixParser::additiveExpression(std::ostream& os) {
    Node* node = multiplicativeExpression(os); // Start with the highest precedence expressions
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
            Token op = currentToken();
            currentTokenIndex++; // Consume the operator token

            right = multiplicativeExpression(os); // Parse the next multiplicative expression

            Node* additiveNode = new Node(op.type == TokenType::ADD ? NodeType::ADD : NodeType::SUBTRACT);
            additiveNode->children.push_back(node);
            additiveNode->children.push_back(right);

            node = additiveNode; // Update node to the newly created one for subsequent loops
            right = nullptr; // Reset right to nullptr to avoid deletion in case of an exception
        }
    } catch (...) {
        clearTree(right); // Clean up the right child if it was created
        clearTree(node); // Clean up everything else
        throw; // Re-throw the exception to be handled further up the stack
    }

    return node; // Return the root of the constructed subtree for additive expressions
}


Node* InfixParser::multiplicativeExpression(std::ostream& os) {
    Node* node = factor(os); // Get the first operand
    Node* right = nullptr;

    try {
        while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE || currentToken().type == TokenType::MODULO) {
            Token op = currentToken();
            currentTokenIndex++; // Move past the operator token

            right = factor(os); // Get the next factor (operand)

            NodeType newNodeType; // Determine node type based on token type
            if (op.type == TokenType::MULTIPLY) {
                newNodeType = NodeType::MULTIPLY;
            } else if (op.type == TokenType::DIVIDE) {
                newNodeType = NodeType::DIVIDE;
            } else { // Must be TokenType::MODULO
                newNodeType = NodeType::MODULO;
            }

            Node* newNode = new Node(newNodeType);
            newNode->children.push_back(node);
            newNode->children.push_back(right);

            node = newNode; // The new node becomes the current node for the next iteration
            right = nullptr; // Reset right to nullptr to prevent deletion in case of an exception
        }
    } catch (...) {
        clearTree(right); // Clean up right if it's been allocated
        clearTree(node); // Clean up the entire left side
        throw; // Re-throw the exception
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

Node* InfixParser::parseIfStatement(std::ostream& os) {
    // Expecting current token to be 'if', which should already be consumed
    consume(TokenType::IF, os);

    Node* condition = expression(os);
    consume(TokenType::LEFT_BRACE, os); // Assuming '{' follows the condition
    Node* thenBranch = parseBlock(os);
    consume(TokenType::RIGHT_BRACE, os);

    Node* elseBranch = nullptr;
    if (currentToken().type == TokenType::ELSE) {
        consume(TokenType::ELSE, os);
        if (currentToken().type == TokenType::IF) {
            elseBranch = parseIfStatement(os);
        } else {
            consume(TokenType::LEFT_BRACE, os);
            elseBranch = parseBlock(os);
            consume(TokenType::RIGHT_BRACE, os);
        }
    }

    Node* ifNode = new Node(NodeType::IF_STATEMENT);
    ifNode->condition = condition;
    ifNode->thenBranch = thenBranch;
    ifNode->elseBranch = elseBranch;

    return ifNode;
}

Node* InfixParser::parseWhileStatement(std::ostream& os) {
    consume(TokenType::WHILE, os);

    Node* condition = expression(os);
    consume(TokenType::LEFT_BRACE, os);
    Node* body = parseBlock(os);
    consume(TokenType::RIGHT_BRACE, os);

    Node* whileNode = new Node(NodeType::WHILE_STATEMENT);
    whileNode->condition = condition;
    whileNode->body.push_back(body);

    return whileNode;
}

Node* InfixParser::parsePrintStatement(std::ostream& os) {
    consume(TokenType::PRINT, os);

    Node* expr = expression(os);
    Node* printNode = new Node(NodeType::PRINT_STATEMENT);
    printNode->children.push_back(expr);

    return printNode;
}

Node* InfixParser::parseBlock(std::ostream& os) {
    // Assuming LEFT_BRACE was already consumed before this call
    std::vector<Node*> statements;
    while (currentToken().type != TokenType::RIGHT_BRACE) {
        Node* statementNode = parseStatement(os);
        statements.push_back(statementNode);
    }

    Node* blockNode = new Node(NodeType::BLOCK);
    blockNode->body = statements;

    return blockNode;
}

Node* InfixParser::parseStatement(std::ostream& os) {
    switch (currentToken().type) {
        case TokenType::IF:
            return parseIfStatement(os);
        case TokenType::WHILE:
            return parseWhileStatement(os);
        case TokenType::PRINT:
            return parsePrintStatement(os);
        // Other cases for different statements
        default:
            return expression(os); // Default to expression if not a recognized statement
    }
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