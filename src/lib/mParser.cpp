
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
    else if (match(TokenType::DEF)) {
        return parseFunctionDefinition();
    }
    else if (match(TokenType::RETURN)) {
        return parseReturnStatement();
    }
    else
    {
        
        stmt = parseExpressionStatement();
    }
    return stmt;

}


std::unique_ptr<ASTNode> Parser::parseFunctionDefinition() {
    Token name = consume(TokenType::IDENTIFIER);
    consume(TokenType::LEFT_PAREN);

    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN);
    std::unique_ptr<ASTNode> body = parseBlock();

    return std::make_unique<FunctionNode>(name, std::move(parameters), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    std::unique_ptr<ASTNode> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    consume(TokenType::SEMICOLON);
    return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseCall(std::unique_ptr<ASTNode> callee) {
    std::vector<std::unique_ptr<ASTNode>> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN);
    return std::make_unique<CallNode>(std::move(callee), std::move(arguments));
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
std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    auto expression = parseExpression();

    // Look ahead to see if the next token is a left parenthesis, indicating a function call
    if (peek().type == TokenType::LEFT_PAREN) {
        // Parse the rest of the function call
        advance(); // Consume the LEFT_PAREN
        expression = parseCall(std::move(expression));

        // Now, check for the semicolon
        if (!match(TokenType::SEMICOLON)) {
            throw std::runtime_error("Expected ';' after print statement");
        }
    } else {
        // For other expressions, still expect a semicolon
        if (!match(TokenType::SEMICOLON)) {
            throw std::runtime_error("Expected ';' after print statement");
        }
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

   
    if (peek().type != TokenType::RIGHT_BRACE && 
        peek().type != TokenType::ELSE &&
        !isAtEnd()) {
        consume(TokenType::SEMICOLON);
    }

    return expression;
}

// parse for all expressions
std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

// parses assignment
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    auto node = parseLogicalOr();
    if (match(TokenType::ASSIGN)) {
        if (node->getType() == ASTNode::Type::VariableNode || node->getType() == ASTNode::Type::ArrayLookupNode) {
            auto value = parseAssignment();
            return std::make_unique<AssignmentNode>(std::move(node), std::move(value));
        } else {
            throw std::runtime_error("Runtime error: invalid assignee.");
        }
    }
    return node;
}


std::unique_ptr<ASTNode> Parser::parseArrayLiteral() {
    std::vector<std::unique_ptr<ASTNode>> elements;
    if (!check(TokenType::RBRACK)) {
        do {
            elements.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RBRACK);
    return std::make_unique<ArrayLiteralNode>(std::move(elements));
}


std::unique_ptr<ASTNode> Parser::parseArrayLookup(std::unique_ptr<ASTNode> array) {
    auto index = parseExpression();
    consume(TokenType::RBRACK);
    
    return std::make_unique<ArrayLookupNode>(std::move(array), std::move(index));
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    std::unique_ptr<ASTNode> node;

    if (match(TokenType::NUMBER)) {
        node = std::make_unique<NumberNode>(previous());
    }  else if (match(TokenType::NULL_TOKEN)) {
        return std::make_unique<NullNode>();
    }
    else if (match(TokenType::LEFT_PAREN)) {
        node = parseExpression();
        consume(TokenType::RIGHT_PAREN);
    } else if (match(TokenType::LBRACK)) {
        node = parseArrayLiteral();
    } else if (match(TokenType::IDENTIFIER)) {
        Token identifier = previous();
        if (check(TokenType::LEFT_PAREN)) {
            advance(); // Consume LEFT_PAREN
            node = parseCall(std::make_unique<VariableNode>(identifier));
        } else {
            node = std::make_unique<VariableNode>(identifier);
        }
    } else if (match(TokenType::BOOLEAN_TRUE) || match(TokenType::BOOLEAN_FALSE) || match(TokenType::NULL_TOKEN)) {
        node = std::make_unique<BooleanNode>(previous());
    } else {
        throw std::runtime_error("Unexpected token at line " + std::to_string(tokens[current].line) + " column " + std::to_string(tokens[current].column) + ": " + tokens[current].value);
    }

    // Handle repeated array lookups
    while (check(TokenType::LBRACK)) {
        advance(); // Consume LBRACK
        auto index = parseExpression();
        consume(TokenType::RBRACK);
        node = std::make_unique<ArrayLookupNode>(std::move(node), std::move(index));
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
            // Simply create a BinaryOpNode for the equality/inequality check
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
    throw std::runtime_error("Unexpected token at line " + std::to_string(tokens[current].line) + " column " + std::to_string(tokens[current].column) + ": " + tokens[current].value);
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