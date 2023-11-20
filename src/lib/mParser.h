#ifndef M_PARSER_H
#define M_PARSER_H

#include "Token.h"
#include "ASTNodes.h"
#include <vector>
#include <memory>
#include<ostream>
#include<iostream>

class Parser {
public:
   
    Parser(const std::vector<Token> &tokens);

    
    std::unique_ptr<ASTNode> parse();
    bool isAtEnd() const;
    bool match(TokenType type);
    bool match(const std::initializer_list<TokenType>& types);


private:
    const std::vector<Token>& tokens; 
    size_t current; 

    
  
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();

    
    std::unique_ptr<ASTNode> parseBinaryExpression(int precedence);

   
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parsePrintStatement();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseExpressionStatement();
    std::unique_ptr<ASTNode> parseStatementOrExpression();

    
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseLogicalOr();
    std::unique_ptr<ASTNode> parseLogicalAnd();
    std::unique_ptr<ASTNode> parseEquality();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseAddition();
    std::unique_ptr<ASTNode> parseMultiplication();
    std::unique_ptr<ASTNode> parsePrimary();

    std::unique_ptr<ASTNode>parseFunctionDefinition();
    std::unique_ptr<ASTNode>parseReturnStatement();
    std::unique_ptr<ASTNode> parseCall(std::unique_ptr<ASTNode> callee);

   
    Token consume(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    const Token& peek() const;
    Token previous();
    void synchronize();
};

#endif // PARSER_H
