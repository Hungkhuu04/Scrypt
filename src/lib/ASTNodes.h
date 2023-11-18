#ifndef ASTNODES_H
#define ASTNODES_H

#include "Token.h"
#include <memory>
#include <vector>
#include <string>


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
        BlockNode,
        FunctionNode,
        ReturnNode,
        CallNode,
        NullNode,
        
    };

    ASTNode(Type type) : nodeType(type) {}
    virtual ~ASTNode() = default;

    Type getType() const { return nodeType; }
    virtual ASTNode* clone() const = 0;

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

    ASTNode* clone() const override {
        return new BinaryOpNode(
            op,
            std::unique_ptr<ASTNode>(left ? left->clone() : nullptr),
            std::unique_ptr<ASTNode>(right ? right->clone() : nullptr)
        );
    }
};

// Node for numeric literals
struct NumberNode : ASTNode {
    Token value;

    explicit NumberNode(Token value)
        : ASTNode(Type::NumberNode), value(value) {}

    ASTNode* clone() const override {
        return new NumberNode(value);
    }
};


// Node for boolean literals
struct BooleanNode : ASTNode {
    Token value;

    explicit BooleanNode(Token value)
        : ASTNode(Type::BooleanNode), value(value) {}

    ASTNode* clone() const override {
        return new BooleanNode(value);
    }
};


// Node for variables (identifiers)
struct VariableNode : ASTNode {
    Token identifier;

    explicit VariableNode(Token identifier)
        : ASTNode(Type::VariableNode), identifier(identifier) {}

    ASTNode* clone() const override {
        return new VariableNode(identifier);
    }
};


// Node for assignment statements
struct AssignmentNode : ASTNode {
    Token identifier;
    std::unique_ptr<ASTNode> expression;

    AssignmentNode(Token identifier, std::unique_ptr<ASTNode> expression)
        : ASTNode(Type::AssignmentNode), identifier(identifier), expression(std::move(expression)) {}

    ASTNode* clone() const override {
        return new AssignmentNode(identifier, std::unique_ptr<ASTNode>(expression->clone()));
    }
};

// Node for print statements
struct PrintNode : ASTNode {
    std::unique_ptr<ASTNode> expression;

    explicit PrintNode(std::unique_ptr<ASTNode> expression)
        : ASTNode(Type::PrintNode), expression(std::move(expression)) {}

    ASTNode* clone() const override {
        return new PrintNode(std::unique_ptr<ASTNode>(expression->clone()));
    }
};

struct NullNode : ASTNode {
    NullNode() : ASTNode(Type::NullNode) {}

    ASTNode* clone() const override {
        return new NullNode(*this);
    }
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

    ASTNode* clone() const override {
        return new IfNode(
            std::unique_ptr<ASTNode>(condition ? condition->clone() : nullptr),
            std::unique_ptr<ASTNode>(trueBranch ? trueBranch->clone() : nullptr),
            std::unique_ptr<ASTNode>(falseBranch ? falseBranch->clone() : nullptr)
        );
    }
};

// Node for while loops
struct WhileNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    WhileNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> body)
        : ASTNode(Type::WhileNode), condition(std::move(condition)), body(std::move(body)) {}

    ASTNode* clone() const override {
        return new WhileNode(
            std::unique_ptr<ASTNode>(condition->clone()),
            std::unique_ptr<ASTNode>(body->clone())
        );
    }
};


// Node for block of statements (compound statement)
struct BlockNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode(std::vector<std::unique_ptr<ASTNode>> statements)
        : ASTNode(Type::BlockNode), statements(std::move(statements)) {}

    ASTNode* clone() const override {
        std::vector<std::unique_ptr<ASTNode>> clonedStatements;
        clonedStatements.reserve(statements.size());
        for (const auto& stmt : statements) {
            clonedStatements.push_back(std::unique_ptr<ASTNode>(stmt->clone()));
        }
        return new BlockNode(std::move(clonedStatements));
    }
};



struct FunctionNode : ASTNode {
    Token name;
    std::vector<Token> parameters;
    std::unique_ptr<ASTNode> body;

    // Constructor
    FunctionNode(Token name, std::vector<Token> parameters, std::unique_ptr<ASTNode> body)
        : ASTNode(Type::FunctionNode), name(std::move(name)), parameters(std::move(parameters)), body(std::move(body)) {}

    // Copy constructor
    FunctionNode(const FunctionNode& other)
        : ASTNode(Type::FunctionNode), name(other.name), parameters(other.parameters) {
        if (other.body) {
            body = std::unique_ptr<ASTNode>(other.body->clone());
        }
    }

    // Copy assignment operator
    FunctionNode& operator=(const FunctionNode& other) {
        if (this != &other) {
            name = other.name;
            parameters = other.parameters;
            body = other.body ? std::unique_ptr<ASTNode>(other.body->clone()) : nullptr;
        }
        return *this;
    }

    // Helper function for cloning
    ASTNode* clone() const override {
        return new FunctionNode(*this);
    }
};

struct ReturnNode : ASTNode {
    std::unique_ptr<ASTNode> value;

    explicit ReturnNode(std::unique_ptr<ASTNode> value)
        : ASTNode(Type::ReturnNode), value(std::move(value)) {}

    ASTNode* clone() const override {
        return new ReturnNode(
            std::unique_ptr<ASTNode>(value ? value->clone() : nullptr)
        );
    }
};

struct CallNode : ASTNode {
    std::unique_ptr<ASTNode> callee;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    CallNode(std::unique_ptr<ASTNode> callee, std::vector<std::unique_ptr<ASTNode>> arguments)
        : ASTNode(Type::CallNode), callee(std::move(callee)), arguments(std::move(arguments)) {}

    ASTNode* clone() const override {
        std::vector<std::unique_ptr<ASTNode>> clonedArguments;
        clonedArguments.reserve(arguments.size());
        for (const auto& arg : arguments) {
            clonedArguments.push_back(std::unique_ptr<ASTNode>(arg->clone()));
        }
        return new CallNode(
            std::unique_ptr<ASTNode>(callee->clone()),
            std::move(clonedArguments)
        );
    }
};


#endif
