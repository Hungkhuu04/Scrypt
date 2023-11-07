
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

void mParser::advance() {
    if (currentTokenIndex < static_cast<int>(tokens.size())) {
        currentTokenIndex++;
    }
}


// This function parses an expression and constructs the Abstract Syntax Tree (AST).
// It checks the current token type and adds the corresponding mnode to the AST.
// If the token type is invalid, it outputs an error message.
mNode* mParser::expression(std::ostream& os) {
    mNode* mnode = nullptr;
    try {
        mnode = assignmentExpression(os);
    } catch (...) {
        clearTree(mnode);
        throw;
    }
    return mnode;  
}


mNode* mParser::assignmentExpression(std::ostream& os) {
    mNode* mnode = logicalOrExpression(os);
    mNode* valuemNode = nullptr;

    try {
        Token op = currentToken();
        if (op.type == TokenType::ASSIGN) {
            if (mnode->type != mNodeType::IDENTIFIER) {
                clearTree(mnode);
                throw std::runtime_error("Unexpected token at line " + std::to_string(op.line) + " column " + std::to_string(op.column) + ": " + op.value + "\n");
            }
            advance();
            valuemNode = assignmentExpression(os);  // Handle right-associative behavior

            mNode* assignmNode = new mNode(mNodeType::ASSIGN);
            assignmNode->children.push_back(mnode);
            assignmNode->children.push_back(valuemNode);

            mnode = assignmNode;
            valuemNode = nullptr;  
        }
    } catch (...) {
        clearTree(valuemNode); // valuemNode might have been partially constructed, so clean it up.
        clearTree(mnode);  // Clean up the entire mnode tree
        throw; // Re-throw the current exception
    }

    return mnode;
}



mNode* mParser::logicalOrExpression(std::ostream& os) {
    mNode* mnode = logicalXorExpression(os);
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_OR) {
            advance();
            right = logicalXorExpression(os);

            mNode* newmNode = new mNode(mNodeType::LOGICAL_OR);
            newmNode->children.push_back(mnode);
            newmNode->children.push_back(right);

            mnode = newmNode;
            right = nullptr;
        }
    } catch (...) {
        clearTree(right);
        clearTree(mnode);
        throw;
    }

    return mnode;
}

mNode* mParser::logicalXorExpression(std::ostream& os) {
    mNode* mnode = logicalAndExpression(os); // Start with a lower precedence expression
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_XOR) {
            advance();
            right = logicalAndExpression(os);

            mNode* newmNode = new mNode(mNodeType::LOGICAL_XOR);
            newmNode->children.push_back(mnode);
            newmNode->children.push_back(right);

            mnode = newmNode;
            right = nullptr;
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(mnode);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return mnode;
}



mNode* mParser::logicalAndExpression(std::ostream& os) {
    mNode* mnode = equalityExpression(os);  // Start with a lower precedence level expression
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::LOGICAL_AND) {
            Token op = currentToken();
            advance(); // Consume the '&&' token
            right = equalityExpression(os); // Parse the right-hand operand

            mNode* andmNode = new mNode(mNodeType::LOGICAL_AND);
            andmNode->children.push_back(mnode);
            andmNode->children.push_back(right);

            mnode = andmNode; // This mnode now becomes the left-hand operand for any further LOGICAL_ANDs
            right = nullptr; // The right mnode is now managed by andmNode, clear the pointer without deleting
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(mnode);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return mnode;
}


mNode* mParser::equalityExpression(std::ostream& os) {
    mNode* mnode = relationalExpression(os);  // Start with a higher precedence level expression
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::EQUAL || currentToken().type == TokenType::NOT_EQUAL) {
            Token op = currentToken();
            advance(); // Consume the equality/inequality token

            right = relationalExpression(os);

            mNode* equalitymNode = new mNode(op.type == TokenType::EQUAL ? mNodeType::EQUAL : mNodeType::NOT_EQUAL);
            equalitymNode->children.push_back(mnode);
            equalitymNode->children.push_back(right);

            mnode = equalitymNode; // This mnode now becomes the left-hand operand for any further equality operations
            right = nullptr; // Prevents deleting right in case of an exception
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(mnode);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return mnode;
}


mNode* mParser::relationalExpression(std::ostream& os) {
    mNode* mnode = additiveExpression(os); // Start with a higher precedence level expression
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::LESS || currentToken().type == TokenType::LESS_EQUAL ||
               currentToken().type == TokenType::GREATER || currentToken().type == TokenType::GREATER_EQUAL) {
            Token op = currentToken();
            advance(); // Consume the relational token

            right = additiveExpression(os);

            mNode* relationalmNode = new mNode(
                op.type == TokenType::LESS ? mNodeType::LESS_THAN :
                op.type == TokenType::LESS_EQUAL ? mNodeType::LESS_EQUAL :
                op.type == TokenType::GREATER ? mNodeType::GREATER_THAN :
                mNodeType::GREATER_EQUAL); // The last condition must be GREATER_EQUAL
            relationalmNode->children.push_back(mnode);
            relationalmNode->children.push_back(right);

            mnode = relationalmNode; // This mnode now becomes the left-hand operand for any further relational operations
            right = nullptr; // Prevents deleting right in case of an exception
        }
    } catch (...) {
        clearTree(right); // right might have been partially constructed, so clean it up.
        clearTree(mnode);  // Clean up the entire left side tree
        throw; // Re-throw the current exception
    }

    return mnode;
}


mNode* mParser::additiveExpression(std::ostream& os) {
    mNode* mnode = multiplicativeExpression(os); // Start with the highest precedence expressions
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
            Token op = currentToken();
            advance(); // Consume the operator token

            right = multiplicativeExpression(os); // Parse the next multiplicative expression

            mNode* additivemNode = new mNode(op.type == TokenType::ADD ? mNodeType::ADD : mNodeType::SUBTRACT);
            additivemNode->children.push_back(mnode);
            additivemNode->children.push_back(right);

            mnode = additivemNode; // Update mnode to the newly created one for subsequent loops
            right = nullptr; // Reset right to nullptr to avoid deletion in case of an exception
        }
    } catch (...) {
        clearTree(right); // Clean up the right child if it was created
        clearTree(mnode); // Clean up everything else
        throw; // Re-throw the exception to be handled further up the stack
    }

    return mnode; // Return the root of the constructed subtree for additive expressions
}


mNode* mParser::multiplicativeExpression(std::ostream& os) {
    mNode* mnode = factor(os); // Get the first operand
    mNode* right = nullptr;

    try {
        while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVIDE || currentToken().type == TokenType::MODULO) {
            Token op = currentToken();
            advance(); // Move past the operator token

            right = factor(os); // Get the next factor (operand)

            mNodeType newmNodeType; // Determine mnode type based on token type
            if (op.type == TokenType::MULTIPLY) {
                newmNodeType = mNodeType::MULTIPLY;
            } else if (op.type == TokenType::DIVIDE) {
                newmNodeType = mNodeType::DIVIDE;
            } else { // Must be TokenType::MODULO
                newmNodeType = mNodeType::MODULO;
            }

            mNode* newmNode = new mNode(newmNodeType);
            newmNode->children.push_back(mnode);
            newmNode->children.push_back(right);

            mnode = newmNode; // The new mnode becomes the current mnode for the next iteration
            right = nullptr; // Reset right to nullptr to prevent deletion in case of an exception
        }
    } catch (...) {
        clearTree(right); // Clean up right if it's been allocated
        clearTree(mnode); // Clean up the entire left side
        throw; // Re-throw the exception
    }

    return mnode;
}

mNode* mParser::factor(std::ostream& os) {
    Token& token = currentToken();
    mNode* mnode = nullptr;  // Initialize mnode pointer to nullptr

    try {
        switch (token.type) {
            case TokenType::NUMBER: {
                mnode = new mNode(mNodeType::NUMBER, std::stod(token.value));
                advance();
                break;
            }
            case TokenType::IDENTIFIER: {
                mnode = new mNode(mNodeType::IDENTIFIER, 0, false, token.value);
                advance();
                break;
            }
            case TokenType::LEFT_PAREN: {
                // Scope for LEFT_PAREN
                unmatchedParentheses++;
                advance();
                mnode = expression(os);
                if (currentToken().type != TokenType::RIGHT_PAREN) {
                    clearTree(mnode); // Clear the current sub-expression
                    throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
                }
                unmatchedParentheses--;
                advance();
                break;
            }
            case TokenType::BOOLEAN_TRUE: {
                mnode = new mNode(mNodeType::BOOLEAN_LITERAL, 1);
                advance();
                break;
            }
            case TokenType::BOOLEAN_FALSE: {
                mnode = new mNode(mNodeType::BOOLEAN_LITERAL, 0);
                advance();
                break;
            }
            default: {
                // Handle unexpected tokens
                throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
            }
        }
    } catch (...) {
        clearTree(mnode); // Clear up any memory allocated before re-throwing
        throw;  // Re-throw the exception to be handled further up the call stack
    }
    return mnode;
}


// This function initiates the parsing process and returns the root of the AST.
mNode* mParser::parse(std::ostream& os) {
    root = new mNode(mNodeType::BLOCK); // Consider the entire program a block
    while (currentTokenIndex < tokens.size() && currentToken().type != TokenType::END) {
        root->children.push_back(parseStatement(os));
    }
    return root;
}

mNode* mParser::parseStatement(std::ostream& os) {
    mNode* node = nullptr;

    // Assume currentToken(), match(), and expression() functions are defined.
    // They manage the current parsing position and parse expressions respectively.
    Token tok = currentToken();

    switch (tok.type) {
        case TokenType::IF:
            node = parseIfStatement(os);
            break;
        case TokenType::WHILE:
            node = parseWhileStatement(os);
            break;
        case TokenType::PRINT:
            node = parsePrintStatement(os);
            break;
        case TokenType::LEFT_BRACE:
            node = parseBlock(os);
            break;
        default:
            node = expression(os);  // Defaults to parsing an expression
            break;
    }

    return node;
}


mNode* mParser::parseIfStatement(std::ostream& os) {
    // Check for 'if' token
    if (tokens[currentTokenIndex].type != TokenType::IF) {
        // Handle parse error: Expected 'if'
    }
    advance(); // Move past 'if'

    mNode* ifNode = new mNode(mNodeType::IF_STATEMENT);
    
    // Parse the condition expression
    ifNode->condition = expression(os);

    // Check for '{' token
    if (tokens[currentTokenIndex].type != TokenType::LEFT_BRACE) {
        // Handle parse error: Expected '{'
    }
    advance(); // Move past '{'

    // Parse the braced block
    ifNode->thenBranch = parseBlock(os);

    // Check for optional 'else'
    if (tokens[currentTokenIndex].type == TokenType::ELSE) {
        advance(); // Move past 'else'

        // Check if we have another 'if' or a braced block
        if (tokens[currentTokenIndex].type == TokenType::IF) {
            ifNode->elseBranch = parseIfStatement(os);
        } else if (tokens[currentTokenIndex].type == TokenType::LEFT_BRACE) {
            ifNode->elseBranch = parseBlock(os);
        } else {
            // Handle parse error: Expected 'if' or '{'
        }
    }

    return ifNode;
}

mNode* mParser::parseWhileStatement(std::ostream& os) {
    // Check for 'while' token
    if (tokens[currentTokenIndex].type != TokenType::WHILE) {
        // Handle parse error: Expected 'while'
    }
    advance(); // Move past 'while'

    mNode* whileNode = new mNode(mNodeType::WHILE_STATEMENT);
    
    // Parse the condition expression
    whileNode->condition = expression(os);

    // Check for '{' token
    if (tokens[currentTokenIndex].type != TokenType::LEFT_BRACE) {
        // Handle parse error: Expected '{'
    }
    advance(); // Move past '{'

    // Parse the braced block
    whileNode->body = parseBlock(os);

    return whileNode;
}

mNode* mParser::parsePrintStatement(std::ostream& os) {
    // Check for 'print' token
    if (tokens[currentTokenIndex].type != TokenType::PRINT) {
        // Handle parse error: Expected 'print'
    }
    advance(); // Move past 'print'

    mNode* printNode = new mNode(mNodeType::PRINT_STATEMENT);
    
    // Parse the expression to be printed
    printNode->Lvalue = expression(os);

    return printNode;
}

mNode* mParser::parseBlock(std::ostream& os) {
    // Check for '{' token
    if (tokens[currentTokenIndex].type != TokenType::LEFT_BRACE) {
        // Handle parse error: Expected '{'
    }
    advance(); // Move past '{'

    mNode* blockNode = new mNode(mNodeType::BLOCK);

    while (tokens[currentTokenIndex].type != TokenType::RIGHT_BRACE) {
        blockNode->statements.push_back(parseStatement(os));
    }

    advance(); // Move past '}'
    
    return blockNode;
}



// This function recursively deallocates memory used by the mnodes in the AST, ensuring no memory leaks.
void mParser::clearTree(mNode*& mnode) {  // Changed mnode to a reference to a pointer
    if (!mnode) return;
    for (mNode*& child : mnode->children) {
        clearTree(child);
    }
    delete mnode;
    mnode = nullptr; 
}


// Destructor for the Parser class. It ensures that the memory used by the AST is deallocated.
mParser::~mParser() {
    clearTree(root);
}