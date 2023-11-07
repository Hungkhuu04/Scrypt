
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
            currentTokenIndex++;
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
            currentTokenIndex++;
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
            currentTokenIndex++;
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
            currentTokenIndex++; // Consume the '&&' token
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
            currentTokenIndex++; // Consume the equality/inequality token

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
            currentTokenIndex++; // Consume the relational token

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
            currentTokenIndex++; // Consume the operator token

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
            currentTokenIndex++; // Move past the operator token

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
                currentTokenIndex++;
                break;
            }
            case TokenType::IDENTIFIER: {
                mnode = new mNode(mNodeType::IDENTIFIER, 0, token.value);
                currentTokenIndex++;
                break;
            }
            case TokenType::LEFT_PAREN: {
                // Scope for LEFT_PAREN
                unmatchedParentheses++;
                currentTokenIndex++;
                mnode = expression(os);
                if (currentToken().type != TokenType::RIGHT_PAREN) {
                    clearTree(mnode); // Clear the current sub-expression
                    throw std::runtime_error("Unexpected token at line " + std::to_string(currentToken().line) + " column " + std::to_string(currentToken().column) + ": " + currentToken().value + "\n");
                }
                unmatchedParentheses--;
                currentTokenIndex++;
                break;
            }
            case TokenType::BOOLEAN_TRUE: {
                mnode = new mNode(mNodeType::BOOLEAN_LITERAL, 1);
                currentTokenIndex++;
                break;
            }
            case TokenType::BOOLEAN_FALSE: {
                mnode = new mNode(mNodeType::BOOLEAN_LITERAL, 0);
                currentTokenIndex++;
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

mNode* mParser::parse(std::ostream& os) {
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
mNode* mParser::parse(std::ostream& os) {
    mNode* rootmNode = new mNode(mNodeType::BLOCK); // A root mnode to hold a sequence of statements
    try {
        while (currentToken().type != TokenType::END) {
            mNode* statementmNode = statement(os);
            if (statementmNode) {
                rootmNode->children.push_back(statementmNode);
            }

            // Enforce newline after each statement, if that is a language requirement.
            if (currentToken().type != TokenType::NEWLINE && currentToken().type != TokenType::END) {
                clearTree(rootmNode);
                throw std::runtime_error("Expected newline after statement at line " +
                                         std::to_string(currentToken().line) + " column " +
                                         std::to_string(currentToken().column));
            }

            // Skip over newlines that separate statements.
            while (currentToken().type == TokenType::NEWLINE) {
                mNode* newlinemNode = new mNode(mNodeType::NEWLINE);
                rootmNode->children.push_back(newlinemNode); // Add the NEWLINE mnode to the AST.
                currentTokenIndex++; // Consume the newline token
            }
        }

        // After processing all statements, check for unmatched parentheses or braces
        if (unmatchedParentheses != 0) {
            clearTree(rootmNode);
            throw std::runtime_error("Unmatched parentheses in input.");
        }
    } catch (const std::runtime_error& e) {
        clearTree(rootmNode); // Clear the AST in case of an exception
        throw; // Re-throw the exception
    }
    return rootmNode;
}

// You may want to introduce a new function to differentiate between expressions and statements
mNode* mParser::statement(std::ostream& os) {
    Token& token = currentToken();
    mNode* mnode = nullptr;

    switch (token.type) {
        case TokenType::IF:
            mnode = ifStatement(os);
            break;
        case TokenType::WHILE:
            mnode = whileStatement(os);
            break;
        case TokenType::PRINT:
            mnode = printStatement(os);
            break;
        case TokenType::IDENTIFIER:
            // If an identifier is encountered, this could be an assignment statement
            mnode = assignmentStatement(os);
            break;
        // ... potentially other statement types ...
        default:
            // If it's not a recognized statement type, it could be an expression
            mnode = expression(os);
            break;
    }
    return mnode;
}

mNode* mParser::assignmentStatement(std::ostream& os) {
    // Current token should be the identifier on the left-hand side of the assignment
    Token& token = currentToken();
    if (token.type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected identifier on the left-hand side of assignment.");
    }

    mNode* assignmNode = new mNode(mNodeType::ASSIGN);
    assignmNode->children.push_back(new mNode(mNodeType::IDENTIFIER, 0, token.value));

    currentTokenIndex++; // Consume the identifier

    // Expect and consume the equals sign
    if (currentToken().type != TokenType::EQUAL) {
        clearTree(assignmNode);
        throw std::runtime_error("Expected '=' after identifier in assignment statement.");
    }
    currentTokenIndex++;

    // Parse the right-hand side expression
    mNode* rhs = expression(os);
    if (!rhs) {
        clearTree(assignmNode);
        throw std::runtime_error("Expected expression after '=' in assignment statement.");
    }
    assignmNode->children.push_back(rhs);

    return assignmNode;
}


mNode* mParser::printStatement(std::ostream& os) {
    currentTokenIndex++; // Consume 'print' token

    mNode* printmNode = new mNode(mNodeType::PRINT);
    mNode* exprmNode = expression(os); // Parse the expression to be printed
    if (!exprmNode) {
        clearTree(printmNode);
        throw std::runtime_error("Expected expression after 'print' keyword.");
    }

    printmNode->children.push_back(exprmNode);
    return printmNode;
}

mNode* mParser::ifStatement(std::ostream& os) {
    mNode* ifmNode = new mNode(mNodeType::IF);

    currentTokenIndex++; // Consume 'if' token

    // Parse the condition
    mNode* condition = expression(os);
    if (!condition) {
        clearTree(ifmNode);
        throw std::runtime_error("Expected condition after 'if' keyword.");
    }
    ifmNode->children.push_back(condition);

    // Parse the 'then' block
    ifmNode->children.push_back(parseBlock(os));

    // Check for an optional 'else'
    if (currentToken().type == TokenType::ELSE) {
        currentTokenIndex++; // Consume 'else' token
        ifmNode->children.push_back(parseBlock(os));
    }

    return ifmNode;
}

mNode* mParser::parseBlock(std::ostream& os) {
    // Expect and consume the left brace '{'
    if (currentToken().type != TokenType::LEFT_BRACE) {
        throw std::runtime_error("Expected '{' at the start of the block.");
    }
    currentTokenIndex++;

    mNode* blockmNode = new mNode(mNodeType::BLOCK);
    // Parse statements inside the block until a '}' is encountered
    while (currentToken().type != TokenType::RIGHT_BRACE && currentToken().type != TokenType::END) {
        mNode* stmt = statement(os);
        if (stmt) {
            blockmNode->children.push_back(stmt);
        }
        // Additional check for END to avoid an infinite loop
        if (currentToken().type == TokenType::END) {
            clearTree(blockmNode);
            throw std::runtime_error("Unexpected end of input. Expected '}' to close the block.");
        }
    }

    if (currentToken().type != TokenType::RIGHT_BRACE) {
        clearTree(blockmNode);
        throw std::runtime_error("Expected '}' at the end of the block.");
    }
    currentTokenIndex++; // Consume the '}' token

    return blockmNode;
}

mNode* mParser::whileStatement(std::ostream& os) {
    // Create a while statement mnode
    mNode* whilemNode = new mNode(mNodeType::WHILE);

    currentTokenIndex++; // Consume the 'while' token

    // Parse the condition
    mNode* condition = expression(os);
    if (!condition) {
        clearTree(whilemNode);
        throw std::runtime_error("Expected condition after 'while' keyword.");
    }
    whilemNode->children.push_back(condition);

    // Expect and consume the left brace '{'
    if (currentToken().type != TokenType::LEFT_BRACE) {
        clearTree(whilemNode);
        throw std::runtime_error("Expected '{' after condition in 'while' statement.");
    }
    currentTokenIndex++;

    // Parse the body of the while loop
    mNode* body = new mNode(mNodeType::BLOCK);
    while (currentToken().type != TokenType::RIGHT_BRACE && currentToken().type != TokenType::END) {
        mNode* stmt = statement(os);
        if (stmt) {
            body->children.push_back(stmt);
        }
        // Additional check for END to avoid an infinite loop
        if (currentToken().type == TokenType::END) {
            clearTree(whilemNode);
            os << "Error: Unexpected end of file. Expected '}' to close 'while' statement.\n";
            return nullptr; // Or throw an exception based on your error handling strategy
        }
    }

    // If the loop exited because it encountered a '}', the body is properly closed
    if (currentToken().type == TokenType::RIGHT_BRACE) {
        whilemNode->children.push_back(body);
        currentTokenIndex++; // Consume the '}' token
    } else {
        // If the loop exited because of an END token, the body wasn't properly closed
        clearTree(whilemNode);
        throw std::runtime_error("Unexpected end of input: expected '}' to close 'while' loop.");
    }

    return whilemNode;
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