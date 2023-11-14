
#include "mParser.h"
#include <iostream>
#include <ostream>

// Constructor
Parser::Parser(const std::vector<Token> &tokens)
        : tokens(tokens), current(0) {}

// Parse the tokens and return the root node of the AST
std::unique_ptr<ASTNode> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;

    while (!isAtEnd()) {
        try {
            std::unique_ptr<ASTNode> stmt = parseStatement();
            if (stmt != nullptr) {
                statements.push_back(std::move(stmt));
            }
        } catch (...) {
            synchronize();
            throw;
        }
    }

    return std::make_unique<BlockNode>(std::move(statements));
}


// Parse function for each rule
std::unique_ptr<ASTNode> Parser::parseStatement()
{
    std::unique_ptr<ASTNode> stmt;   
    if (match(TokenType::IF))
    {
        stmt = parseIfStatement();
    }
    else if (match(TokenType::WHILE))
    {
        stmt = parseWhileStatement();
    }
    else if (match(TokenType::PRINT))
    {
        stmt = parsePrintStatement();
    }
    else if (match(TokenType::LEFT_BRACE))
    {
        stmt = parseBlock();
    }
    else
    {
        
        stmt = parseExpressionStatement();
    }
    return stmt;

}

// Parses if statements & blocks
std::unique_ptr<ASTNode> Parser::parseIfStatement()
{
    
    auto condition = parseExpression();
    
    auto trueBranch = parseBlock();
    
    std::unique_ptr<ASTNode> elseBranch = nullptr;
    if (match(TokenType::ELSE))
    {
        while (match(TokenType::NEWLINE)) {  
        }
       

        if (check(TokenType::IF))
        {
            advance();
            elseBranch = parseIfStatement();
        }
        else
        {
            elseBranch = parseBlock();
        }

    }
    return std::make_unique<IfNode>(std::move(condition), std::move(trueBranch), std::move(elseBranch));
}

// parses while statements and blocks
std::unique_ptr<ASTNode> Parser::parseWhileStatement()
{
    
    while (match(TokenType::NEWLINE))
    {
        //  consuming the newline
    }

    auto condition = parseExpression(); 

   
    while (match(TokenType::NEWLINE))
    {
        //  consuming the newline
    }

    
    auto body = parseBlock();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

// parses print statements
std::unique_ptr<ASTNode> Parser::parsePrintStatement()
{
    auto expression = parseExpression();
    while (match(TokenType::NEWLINE))
    {
        // consuming the newline
    }
    return std::make_unique<PrintNode>(std::move(expression));
}

// parses block nodes
std::unique_ptr<ASTNode> Parser::parseBlock()
{
    consume(TokenType::LEFT_BRACE);
    std::vector<std::unique_ptr<ASTNode>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        while (match(TokenType::NEWLINE))
        {
            // consuming the newline
        }
        statements.push_back(parseStatement());
        
    }

    consume(TokenType::RIGHT_BRACE);
    while (match(TokenType::NEWLINE))
    {
        // consuming the newline
    }
    
    return std::make_unique<BlockNode>(std::move(statements));
}

// main parse for expression statements (NOT BLOCKS)
std::unique_ptr<ASTNode> Parser::parseExpressionStatement()
{
    auto expression = parseExpression();

   
    while (match(TokenType::NEWLINE)) {
            //  consuming the newline
    }

    return expression;
}

// parse for all expressions
std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

// parses assignment
std::unique_ptr<ASTNode> Parser::parseAssignment()
{
    auto node = parseLogicalOr();
    if (match(TokenType::ASSIGN))
    {
        Token equals = previous();
        auto value = parseAssignment();
        if (node->getType() != ASTNode::Type::VariableNode)
        {
            throw error();
        }
        auto variable = static_cast<VariableNode *>(node.get());
        return std::make_unique<AssignmentNode>(variable->identifier, std::move(value));
    }
    return node;
}


/* From here to parse primary, each function parses each type of logical operation or expresion. 
It is coded to use precedence from track A
*/
std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    try {
        auto node = parseLogicalAnd();
        while (match(TokenType::LOGICAL_OR)) {
            Token op = previous();
            auto right = parseLogicalAnd();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    try {
        auto node = parseEquality();
        while (match(TokenType::LOGICAL_AND)) {
            Token op = previous();
            auto right = parseEquality();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    try {
        auto node = parseComparison();
        while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
            Token op = previous();
            auto right = parseComparison();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}



std::unique_ptr<ASTNode> Parser::parseAddition() {
    try {
        auto node = parseMultiplication();
        while (match(TokenType::ADD) || match(TokenType::SUBTRACT)) {
            Token op = previous();
            auto right = parseMultiplication();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}

std::unique_ptr<ASTNode> Parser::parseComparison() {
    try {
        auto node = parseAddition();
        while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) ||
               match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL)) {
            Token op = previous();
            auto right = parseAddition();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}

std::unique_ptr<ASTNode> Parser::parseMultiplication() {
    try {
        auto node = parsePrimary();
        while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO)) {
            Token op = previous();
            auto right = parsePrimary();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }
        return node;
    } catch (...) {
        throw;
    }
}


std::unique_ptr<ASTNode> Parser::parsePrimary() {
    try {
        if (match(TokenType::NUMBER)) {
            return std::make_unique<NumberNode>(previous());
        } else if (match(TokenType::LEFT_PAREN)) {
            auto expr = parseExpression();
            consume(TokenType::RIGHT_PAREN);
            while (match(TokenType::NEWLINE)) {
            }
            return expr;
        } else if (match(TokenType::IDENTIFIER)) {
            return std::make_unique<VariableNode>(previous());
        } else if (match(TokenType::BOOLEAN_TRUE)) {
            return std::make_unique<BooleanNode>(previous());
        } else if (match(TokenType::BOOLEAN_FALSE)) {
            return std::make_unique<BooleanNode>(previous());
        }
    } catch (...) {
        throw error();
    }
    throw error();
}



//Retrieves the current token without advancing the parser
const Token &Parser::peek() const
{
    if (isAtEnd())
    {
        static const Token eofToken(TokenType::END, "END", tokens[current].line, tokens[current].column); 
        return eofToken;
    }
    return tokens[current];
}

// Advances the parser until a stopping point of a newline
void Parser::synchronize()
{
    advance();
    while (!isAtEnd())
    {
        if (previous().type == TokenType::NEWLINE)
            return;
        switch (peek().type)
        {
        
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
            return;
        default:
            advance();
        }
    }
}

//Checks and advances if the current token matches the given type
bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

//Retrieves the previous token
Token Parser::previous()
{
    return tokens.at(current - 1);
}

//Consumes a token of the given type
Token Parser::consume(TokenType type)
{
    if (check(type)) 
        return advance();
    throw error();
}

//Checks if the current token is of the given type.
bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return false;
    return tokens.at(current).type == type;
}

//Advances to the next token
Token Parser::advance()
{
    if (!isAtEnd())
        current++;

    
    Token token = previous();

    
    return token;
}

//Checks if the parser has reached the end of the tokens.
bool Parser::isAtEnd() const
{
    return current >= tokens.size() || tokens.at(current).type == TokenType::END;
}

//Throws a runtime error indicating an unexpected token.
ParseError Parser::error() {
    throw std::runtime_error("Unexpected token at line " + std::to_string(tokens[current].line) + " column " + std::to_string(tokens[current].column) + ": " + tokens[current].value);
}