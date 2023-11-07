
// Include the header file "parse.h" which likely contains the declarations for the Parser class and its methods.
#include "mParser.h"
#include <iostream>    
#include <string>      
#include <stdexcept>

// Constructor for the Parser class. Initializes the tokens vector and sets the current token index to 0.
mParser::mParser(const std::vector<Token>& tokens) 
    : tokens(tokens), currentTokenIndex(0), root(nullptr), unmatchedParentheses(0) {}


// Returns the current token being processed.
Token& mParser::currentToken() {
    return tokens[currentTokenIndex];
}


// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding node to the AST.
// If the token type is invalid, it outputs an error message.
Node* mParser::expression(std::ostream& os) {
    Node* node = nullptr;
    try {
        node = assignmentExpression(os);
    } catch (...) {
        clearTree(node);
        throw;
    }
    return node;  
}


Node* mParser::assignmentExpression(std::ostream& os) {
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



Node* mParser::logicalOrExpression(std::ostream& os) {
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

Node* mParser::logicalXorExpression(std::ostream& os) {
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



Node* mParser::logicalAndExpression(std::ostream& os) {
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


Node* mParser::equalityExpression(std::ostream& os) {
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


Node* mParser::relationalExpression(std::ostream& os) {
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


Node* mParser::additiveExpression(std::ostream& os) {
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


Node* mParser::multiplicativeExpression(std::ostream& os) {
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

Node* mParser::factor(std::ostream& os) {
    Token& token = currentToken();
    Node* node = nullptr;  // Initialize node pointer to nullptr

    try {
        switch (token.type) {
            case TokenType::NUMBER: {
                node = new Node(NodeType::NUMBER, std::stod(token.value));
                currentTokenIndex++;
                break;
            }
            case TokenType::IDENTIFIER: {
                node = new Node(NodeType::IDENTIFIER, 0, token.value);
                currentTokenIndex++;
                break;
            }
            case TokenType::LEFT_PAREN: {
                // Scope for LEFT_PAREN
                unmatchedParentheses++;
                currentTokenIndex++;
                node = expression(os);
                if (currentToken().type != TokenType::RIGHT_PAREN) {
                    clearTree(node); // Clear the current sub-expression
                    throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
                }
                unmatchedParentheses--;
                currentTokenIndex++;
                break;
            }
            case TokenType::BOOLEAN_TRUE: {
                node = new Node(NodeType::BOOLEAN_LITERAL, 1);
                currentTokenIndex++;
                break;
            }
            case TokenType::BOOLEAN_FALSE: {
                node = new Node(NodeType::BOOLEAN_LITERAL, 0);
                currentTokenIndex++;
                break;
            }
            default: {
                // Handle unexpected tokens
                throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
            }
        }
    } catch (...) {
        clearTree(node); // Clear up any memory allocated before re-throwing
        throw;  // Re-throw the exception to be handled further up the call stack
    }
    return node;
}

Node* mParser::parse(std::ostream& os) {
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
        throw;  // Re-throw the caught exception
    }
    return root;
}


// This function initiates the parsing process and returns the root of the AST.
Node* mParser::parse(std::ostream& os) {
    Node* rootNode = new Node(NodeType::BLOCK); // A root node to hold a sequence of statements
    try {
        while (currentToken().type != TokenType::END) {
            Node* statementNode = statement(os);
            if (statementNode) {
                rootNode->children.push_back(statementNode);
            }

            // Enforce newline after each statement, if that is a language requirement.
            if (currentToken().type != TokenType::NEWLINE && currentToken().type != TokenType::END) {
                clearTree(rootNode);
                throw std::runtime_error("Expected newline after statement at line " +
                                         std::to_string(currentToken().line) + " column " +
                                         std::to_string(currentToken().column));
            }

            // Skip over newlines that separate statements.
            while (currentToken().type == TokenType::NEWLINE) {
                Node* newlineNode = new Node(NodeType::NEWLINE);
                rootNode->children.push_back(newlineNode); // Add the NEWLINE node to the AST.
                currentTokenIndex++; // Consume the newline token
            }
        }

        // After processing all statements, check for unmatched parentheses or braces
        if (unmatchedParentheses != 0) {
            clearTree(rootNode);
            throw std::runtime_error("Unmatched parentheses in input.");
        }
    } catch (const std::runtime_error& e) {
        clearTree(rootNode); // Clear the AST in case of an exception
        throw; // Re-throw the exception
    }
    return rootNode;
}

// You may want to introduce a new function to differentiate between expressions and statements
Node* mParser::statement(std::ostream& os) {
    Token& token = currentToken();
    Node* node = nullptr;

    switch (token.type) {
        case TokenType::IF:
            node = ifStatement(os);
            break;
        case TokenType::WHILE:
            node = whileStatement(os);
            break;
        case TokenType::PRINT:
            node = printStatement(os);
            break;
        case TokenType::IDENTIFIER:
            // If an identifier is encountered, this could be an assignment statement
            node = assignmentStatement(os);
            break;
        // ... potentially other statement types ...
        default:
            // If it's not a recognized statement type, it could be an expression
            node = expression(os);
            break;
    }
    return node;
}

Node* mParser::assignmentStatement(std::ostream& os) {
    // Current token should be the identifier on the left-hand side of the assignment
    Token& token = currentToken();
    if (token.type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected identifier on the left-hand side of assignment.");
    }

    Node* assignNode = new Node(NodeType::ASSIGN);
    assignNode->children.push_back(new Node(NodeType::IDENTIFIER, 0, token.value));

    currentTokenIndex++; // Consume the identifier

    // Expect and consume the equals sign
    if (currentToken().type != TokenType::EQUAL) {
        clearTree(assignNode);
        throw std::runtime_error("Expected '=' after identifier in assignment statement.");
    }
    currentTokenIndex++;

    // Parse the right-hand side expression
    Node* rhs = expression(os);
    if (!rhs) {
        clearTree(assignNode);
        throw std::runtime_error("Expected expression after '=' in assignment statement.");
    }
    assignNode->children.push_back(rhs);

    return assignNode;
}


Node* mParser::printStatement(std::ostream& os) {
    currentTokenIndex++; // Consume 'print' token

    Node* printNode = new Node(NodeType::PRINT);
    Node* exprNode = expression(os); // Parse the expression to be printed
    if (!exprNode) {
        clearTree(printNode);
        throw std::runtime_error("Expected expression after 'print' keyword.");
    }

    printNode->children.push_back(exprNode);
    return printNode;
}

Node* mParser::ifStatement(std::ostream& os) {
    Node* ifNode = new Node(NodeType::IF);

    currentTokenIndex++; // Consume 'if' token

    // Parse the condition
    Node* condition = expression(os);
    if (!condition) {
        clearTree(ifNode);
        throw std::runtime_error("Expected condition after 'if' keyword.");
    }
    ifNode->children.push_back(condition);

    // Parse the 'then' block
    ifNode->children.push_back(parseBlock(os));

    // Check for an optional 'else'
    if (currentToken().type == TokenType::ELSE) {
        currentTokenIndex++; // Consume 'else' token
        ifNode->children.push_back(parseBlock(os));
    }

    return ifNode;
}

Node* mParser::parseBlock(std::ostream& os) {
    // Expect and consume the left brace '{'
    if (currentToken().type != TokenType::LEFT_BRACE) {
        throw std::runtime_error("Expected '{' at the start of the block.");
    }
    currentTokenIndex++;

    Node* blockNode = new Node(NodeType::BLOCK);
    // Parse statements inside the block until a '}' is encountered
    while (currentToken().type != TokenType::RIGHT_BRACE && currentToken().type != TokenType::END) {
        Node* stmt = statement(os);
        if (stmt) {
            blockNode->children.push_back(stmt);
        }
        // Additional check for END to avoid an infinite loop
        if (currentToken().type == TokenType::END) {
            clearTree(blockNode);
            throw std::runtime_error("Unexpected end of input. Expected '}' to close the block.");
        }
    }

    if (currentToken().type != TokenType::RIGHT_BRACE) {
        clearTree(blockNode);
        throw std::runtime_error("Expected '}' at the end of the block.");
    }
    currentTokenIndex++; // Consume the '}' token

    return blockNode;
}

Node* mParser::whileStatement(std::ostream& os) {
    // Create a while statement node
    Node* whileNode = new Node(NodeType::WHILE);

    currentTokenIndex++; // Consume the 'while' token

    // Parse the condition
    Node* condition = expression(os);
    if (!condition) {
        clearTree(whileNode);
        throw std::runtime_error("Expected condition after 'while' keyword.");
    }
    whileNode->children.push_back(condition);

    // Expect and consume the left brace '{'
    if (currentToken().type != TokenType::LEFT_BRACE) {
        clearTree(whileNode);
        throw std::runtime_error("Expected '{' after condition in 'while' statement.");
    }
    currentTokenIndex++;

    // Parse the body of the while loop
    Node* body = new Node(NodeType::BLOCK);
    while (currentToken().type != TokenType::RIGHT_BRACE && currentToken().type != TokenType::END) {
        Node* stmt = statement(os);
        if (stmt) {
            body->children.push_back(stmt);
        }
        // Additional check for END to avoid an infinite loop
        if (currentToken().type == TokenType::END) {
            clearTree(whileNode);
            os << "Error: Unexpected end of file. Expected '}' to close 'while' statement.\n";
            return nullptr; // Or throw an exception based on your error handling strategy
        }
    }

    // If the loop exited because it encountered a '}', the body is properly closed
    if (currentToken().type == TokenType::RIGHT_BRACE) {
        whileNode->children.push_back(body);
        currentTokenIndex++; // Consume the '}' token
    } else {
        // If the loop exited because of an END token, the body wasn't properly closed
        clearTree(whileNode);
        throw std::runtime_error("Unexpected end of input: expected '}' to close 'while' loop.");
    }

    return whileNode;
}


// This function recursively deallocates memory used by the nodes in the AST, ensuring no memory leaks.
void mParser::clearTree(Node*& node) {  // Changed node to a reference to a pointer
    if (!node) return;
    for (Node*& child : node->children) {
        clearTree(child);
    }
    delete node;
    node = nullptr; 
}


// Destructor for the Parser class. It ensures that the memory used by the AST is deallocated.
mParser::~mParser() {
    clearTree(root);
}