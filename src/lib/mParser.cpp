#include "mParser.h"
#include <iostream>
#include <ostream>

// Constructor implementation
Parser::Parser(const std::vector<Token> &tokens, std::ostream& errorOut)
        : tokens(tokens), current(0), errorOutput(errorOut) {}

// Parse entry function
std::unique_ptr<ASTNode> Parser::parse()
{
    
    std::vector<std::unique_ptr<ASTNode>> statements;

   
    while (!isAtEnd())
    {
        try
        {
            
            statements.push_back(parseStatement());
        }
        catch (const ParseError &error)
        {
    
            synchronize();
        }
    }

    return std::make_unique<BlockNode>(std::move(statements));
}

// Implementations of parsing functions for each rule
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

std::unique_ptr<ASTNode> Parser::parseWhileStatement()
{
    
    while (match(TokenType::NEWLINE))
    {
        // Just consuming the newline
    }

    auto condition = parseExpression(); 

   
    while (match(TokenType::NEWLINE))
    {
        // Just consuming the newline
    }

    
    auto body = parseBlock();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement()
{
    auto expression = parseExpression();
    while (match(TokenType::NEWLINE))
    {
        
    }
    
    return std::make_unique<PrintNode>(std::move(expression));
}

std::unique_ptr<ASTNode> Parser::parseBlock()
{
    consume(TokenType::LEFT_BRACE, "Expect '{' before block.");
    std::vector<std::unique_ptr<ASTNode>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        while (match(TokenType::NEWLINE))
        {
            
        }
        statements.push_back(parseStatement());
        
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    while (match(TokenType::NEWLINE))
    {
            
    }
    
    return std::make_unique<BlockNode>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseExpressionStatement()
{
    auto expression = parseExpression();

   
    while (match(TokenType::NEWLINE)) {
            // Just consuming the newline
    }

    return expression;
}

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

std::unique_ptr<ASTNode> Parser::parseAssignment()
{
    auto node = parseLogicalOr();
    if (match(TokenType::ASSIGN))
    {
        Token equals = previous();
        auto value = parseAssignment();
        if (node->getType() != ASTNode::Type::VariableNode)
        {
            throw errorAt(equals, "");
        }
        auto variable = static_cast<VariableNode *>(node.get());
        return std::make_unique<AssignmentNode>(variable->identifier, std::move(value));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr()
{
    auto node = parseLogicalAnd();
    while (match(TokenType::LOGICAL_OR))
    {
        Token op = previous();
        auto right = parseLogicalAnd();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd()
{
    auto node = parseEquality();
    while (match(TokenType::LOGICAL_AND))
    {
        Token op = previous();
        auto right = parseEquality();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseEquality()
{
    auto node = parseComparison();
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL))
    {
        Token op = previous();
        auto right = parseComparison();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseComparison()
{
    auto node = parseAddition();
    while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) ||
           match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL))
    {
        Token op = previous();
        auto right = parseAddition();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseAddition()
{
    auto node = parseMultiplication();
    while (match(TokenType::ADD) || match(TokenType::SUBTRACT))
    {
        Token op = previous();
        auto right = parseMultiplication();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseMultiplication()
{
    auto node = parsePrimary();
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO))
    {
        Token op = previous();
        auto right = parsePrimary();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}


std::unique_ptr<ASTNode> Parser::parsePrimary()
{
    if (match(TokenType::NUMBER))
    {
        
        return std::make_unique<NumberNode>(previous());
    }
    else if (match(TokenType::LEFT_PAREN))
    {
       
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        while (match(TokenType::NEWLINE)) {
        }
        return expr;
    }
    else if (match(TokenType::IDENTIFIER))
    {
        
        return std::make_unique<VariableNode>(previous());
    }
    else if (match(TokenType::BOOLEAN_TRUE))
    {
        
        return std::make_unique<BooleanNode>(previous());
    }
    else if (match(TokenType::BOOLEAN_FALSE))
    {
        
        return std::make_unique<BooleanNode>(previous());
    }
    
    throw errorAtCurrent("");
}



const Token &Parser::peek() const
{
    if (isAtEnd())
    {
        static const Token eofToken(TokenType::END, "END", tokens[current].line, tokens[current].column); 
        return eofToken;
    }
    return tokens[current];
}

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

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::previous()
{
    return tokens.at(current - 1);
}

Token Parser::consume(TokenType type, const std::string &message)
{
    if (check(type))
        return advance();

    throw errorAtCurrent(message);
}


bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return false;
    return tokens.at(current).type == type;
}


Token Parser::advance()
{
    if (!isAtEnd())
        current++;

    
    Token token = previous();

    
    return token;
}


bool Parser::isAtEnd() const
{
    return current >= tokens.size() || tokens.at(current).type == TokenType::END;
}


void Parser::error(const Token &token, const std::string &message) {
    report(token.line, token.column, token.value, message);
}

ParseError Parser::report(int line, int column, const std::string &tokenValue, const std::string &message) {
    throw std::runtime_error( "Unexpected token at line " + std::to_string(line) + " column " + std::to_string(column) + ": " + tokenValue + "\n");
    return ParseError(message);
}

ParseError Parser::errorAtCurrent(const std::string &message) {
    return errorAt(peek(), message);
}

ParseError Parser::errorAt(const Token &token, const std::string &message) {
    error(token, message);
    return ParseError(message);
}

