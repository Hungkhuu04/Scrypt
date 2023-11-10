#ifndef ASTNODES_H
#define ASTNODES_H

#include "Token.h"
#include <memory>
#include <vector>
#include <string>

class Value {
public:
    enum class Type { Double, Bool } type;
    Value() : type(Type::Double), doubleValue(0) {}
    Value(double value) : type(Type::Double), doubleValue(value) {}
    Value(bool value) : type(Type::Bool), boolValue(value) {}

    double asDouble() const; 
    bool asBool() const;
private:
    union {
        double doubleValue;
        bool boolValue;
    };


};


struct ASTNode {
    enum class Type {
        BinaryOpNode,
        NumberNode,
        BooleanNode,
        VariableNode,
        AssignmentNode,
        PrintNode,
        IfNode,
        WhileNode,
        BlockNode
        
    };

    ASTNode(Type type) : nodeType(type) {}
    virtual ~ASTNode() = default;

    Type getType() const { return nodeType; }

private:
    Type nodeType;
};

// Node for binary operations 
struct BinaryOpNode : ASTNode {
    Token op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(Token op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : ASTNode(Type::BinaryOpNode), op(op), left(std::move(left)), right(std::move(right)) {}
};

// Node for numeric literals
struct NumberNode : ASTNode {
    Token value;

    explicit NumberNode(Token value)
        : ASTNode(Type::NumberNode), value(value) {}
};

// Node for boolean literals
struct BooleanNode : ASTNode {
    Token value;

    explicit BooleanNode(Token value)
        : ASTNode(Type::BooleanNode), value(value) {}
};

// Node for variables (identifiers)
struct VariableNode : ASTNode {
    Token identifier;

    explicit VariableNode(Token identifier)
        : ASTNode(Type::VariableNode), identifier(identifier) {}
};

// Node for assignment statements
struct AssignmentNode : ASTNode {
    Token identifier;
    std::unique_ptr<ASTNode> expression;

    AssignmentNode(Token identifier, std::unique_ptr<ASTNode> expression)
        : ASTNode(Type::AssignmentNode), identifier(identifier), expression(std::move(expression)) {}
};

// Node for print statements
struct PrintNode : ASTNode {
    std::unique_ptr<ASTNode> expression;

    explicit PrintNode(std::unique_ptr<ASTNode> expression)
        : ASTNode(Type::PrintNode), expression(std::move(expression)) {}
};

// Node for if statements
struct IfNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> trueBranch;
    std::unique_ptr<ASTNode> falseBranch;

    IfNode(std::unique_ptr<ASTNode> condition,
           std::unique_ptr<ASTNode> trueBranch,
           std::unique_ptr<ASTNode> falseBranch)
        : ASTNode(Type::IfNode),
          condition(std::move(condition)),
          trueBranch(std::move(trueBranch)),
          falseBranch(std::move(falseBranch)) {}
};

// Node for while loops
struct WhileNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    WhileNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> body)
        : ASTNode(Type::WhileNode), condition(std::move(condition)), body(std::move(body)) {}
};

// Node for block of statements (compound statement)
struct BlockNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode(std::vector<std::unique_ptr<ASTNode>> statements)
        : ASTNode(Type::BlockNode), statements(std::move(statements)) {}
};


#endif
