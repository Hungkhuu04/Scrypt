
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
    return assignmentExpression(os);
}

mNode* mParser::assignmentExpression(std::ostream& os) {
    mNode* node = logicalOrExpression(os);

    if (currentToken().type == TokenType::ASSIGN) {
        advance();  // Consume the '='
        mNode* assignNode = new mNode(mNodeType::ASSIGN);
        assignNode->children.push_back(node);
        assignNode->children.push_back(assignmentExpression(os));  // Right-hand side of the assignment
        node = assignNode;
    }

    return node;
}

mNode* mParser::logicalOrExpression(std::ostream& os) {
    mNode* node = logicalAndExpression(os);

    while (currentToken().type == TokenType::LOGICAL_OR) {
        advance();  // Consume the '||'
        mNode* orNode = new mNode(mNodeType::LOGICAL_OR);
        orNode->children.push_back(node);
        orNode->children.push_back(logicalAndExpression(os));
        node = orNode;
    }

    return node;
}

mNode* mParser::logicalAndExpression(std::ostream& os) {
    mNode* node = equalityExpression(os);

    while (currentToken().type == TokenType::LOGICAL_AND) {
        advance();  // Consume the '&&'
        mNode* andNode = new mNode(mNodeType::LOGICAL_AND);
        andNode->children.push_back(node);
        andNode->children.push_back(equalityExpression(os));
        node = andNode;
    }

    return node;
}

mNode* mParser::equalityExpression(std::ostream& os) {
    mNode* node = relationalExpression(os);

    while (currentToken().type == TokenType::EQUAL || currentToken().type == TokenType::NOT_EQUAL) {
        TokenType operatorType = currentToken().type;
        advance();  // Consume '==' or '!='
        mNode* eqNode = new mNode(operatorType == TokenType::EQUAL ? mNodeType::EQUAL : mNodeType::NOT_EQUAL);
        eqNode->children.push_back(node);
        eqNode->children.push_back(relationalExpression(os));
        node = eqNode;
    }

    return node;
}

mNode* mParser::relationalExpression(std::ostream& os) {
    mNode* node = additiveExpression(os);

    while (currentToken().type == TokenType::LESS || currentToken().type == TokenType::LESS_EQUAL ||
           currentToken().type == TokenType::GREATER || currentToken().type == TokenType::GREATER_EQUAL) {
        TokenType operatorType = currentToken().type;
        advance();  // Consume '<', '<=', '>', '>='
        mNode* relNode = new mNode(
            operatorType == TokenType::LESS ? mNodeType::LESS :
            operatorType == TokenType::LESS_EQUAL ? mNodeType::LESS_EQUAL :
            operatorType == TokenType::GREATER ? mNodeType::GREATER_THAN :
            mNodeType::GREATER_EQUAL
        );
        relNode->children.push_back(node);
        relNode->children.push_back(additiveExpression(os));
        node = relNode;
    }

    return node;
}

mNode* mParser::additiveExpression(std::ostream& os) {
    mNode* node = multiplicativeExpression(os);

    while (currentToken().type == TokenType::ADD || currentToken().type == TokenType::SUBTRACT) {
        TokenType operatorType = currentToken().type;
        advance();  // Consume '+' or '-'
        mNode* addNode = new mNode(operatorType == TokenType::ADD ? mNodeType::ADD : mNodeType::SUBTRACT);
        addNode->children.push_back(node);
        addNode->children.push_back(multiplicativeExpression(os));
        node = addNode;
    }

    return node;
}
mNode* mParser::multiplicativeExpression(std::ostream& os) {
    mNode* node = factor(os);

    while (currentToken().type == TokenType::MULTIPLY || 
           currentToken().type == TokenType::DIVIDE || 
           currentToken().type == TokenType::MODULO) {
        TokenType operatorType = currentToken().type;
        advance();  // Consume '*', '/' or '%'

        mNode* newNode = new mNode(
            operatorType == TokenType::MULTIPLY ? mNodeType::MULTIPLY :
            operatorType == TokenType::DIVIDE ? mNodeType::DIVIDE :
            mNodeType::MODULO
        );

        newNode->children.push_back(node); // The existing node becomes the left operand
        newNode->children.push_back(factor(os)); // Right operand

        node = newNode; // Update the current node to the new compound node
    }

    return node;
}

// The factor(os) function is assumed to be the next part of your expression hierarchy.


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
                advance();  // Consume the '('
                mnode = expression(os);  // Parse the expression within the parentheses
                if (currentToken().type != TokenType::RIGHT_PAREN) {
                    throw std::runtime_error("Expected ')' after expression");
                }
                advance();  // Consume the ')'
                break;
            }
            case TokenType::BOOLEAN_TRUE:
            case TokenType::BOOLEAN_FALSE: {
                mnode = new mNode(mNodeType::BOOLEAN_LITERAL, token.type == TokenType::BOOLEAN_TRUE);
                advance();
                break;
            }
            default: {
                throw std::runtime_error("Unexpected token: " + token.value);
            }
        }
    } catch (...) {
        delete mnode;
        throw;  // Re-throw the exception to be handled further up the call stack
    }
    return mnode;
}



// This function initiates the parsing process and returns the root of the AST.
mNode* mParser::parse(std::ostream& os) {
    root = new mNode(mNodeType::BLOCK); // Consider the entire program a block
    while (currentTokenIndex < static_cast<int>(tokens.size()) && currentToken().type != TokenType::END) {
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