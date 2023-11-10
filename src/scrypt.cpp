#include "lib/mParser.h"
#include "lib/lex.h"
#include "lib/ASTNodes.h" 
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>

std::unordered_map<std::string, Value> variables;
Value evaluateExpression(const ASTNode* node);
void evaluateBlock(const BlockNode* blockNode);
void evaluateIf(ASTNode* node);
void evaluateWhile(const WhileNode* node);
void evaluatePrint(const PrintNode* node);
Value evaluateBinaryOperation(const BinaryOpNode* node);
Value evaluateVariable(const VariableNode* node);
Value evaluateAssignment(const AssignmentNode* assignmentNode);
Value tokenToValue(const Token& token);

Value tokenToValue(const Token& token) {
    switch (token.type) {
        case TokenType::NUMBER:
            return Value(std::stod(token.value));
        case TokenType::BOOLEAN_TRUE:
            return Value(true);
        case TokenType::BOOLEAN_FALSE:
            return Value(false);
        default:
            throw std::runtime_error("Invalid token type for value conversion");
    }
}

// Evaluate the expression node
// Evaluate the expression node
Value evaluateExpression(const ASTNode* node) {
    if (!node) {
        throw std::runtime_error("Null expression node");
    }

    switch (node->getType()) {
        case ASTNode::Type::NumberNode:
            return Value(std::stod(static_cast<const NumberNode*>(node)->value.value));
        case ASTNode::Type::BooleanNode:
            return Value(static_cast<const BooleanNode*>(node)->value.type == TokenType::BOOLEAN_TRUE);
        case ASTNode::Type::VariableNode:
            return evaluateVariable(static_cast<const VariableNode*>(node));
        case ASTNode::Type::BinaryOpNode:
            return evaluateBinaryOperation(static_cast<const BinaryOpNode*>(node));
        case ASTNode::Type::AssignmentNode:
            return evaluateAssignment(static_cast<const AssignmentNode*>(node));
        default:
            throw std::runtime_error("Unknown expression node type");
    }
}

// Evaluate the block node
void evaluateBlock(const BlockNode* blockNode) {
    if (!blockNode) {
        throw std::runtime_error("Null block node passed to evaluateBlock");
    }
    
    for (const auto& stmt : blockNode->statements) {
        switch (stmt->getType()) {
            case ASTNode::Type::IfNode:
                evaluateIf(stmt.get());
                break;

            case ASTNode::Type::WhileNode:
                evaluateWhile(static_cast<const WhileNode*>(stmt.get()));
                break;
            case ASTNode::Type::PrintNode:
                evaluatePrint(static_cast<const PrintNode*>(stmt.get()));
                break;
            case ASTNode::Type::AssignmentNode:
                evaluateAssignment(static_cast<const AssignmentNode*>(stmt.get()));
                break;
            case ASTNode::Type::BlockNode:
                evaluateBlock(static_cast<const BlockNode*>(stmt.get()));
                break;
            default:
                throw std::runtime_error("Unknown Node Type");
        }
    }
}


// Evaluate the if node
void evaluateIf(ASTNode* node) {
    IfNode* ifNode = dynamic_cast<IfNode*>(node);
    if (!ifNode) {
        throw std::runtime_error("Non-if node passed to evaluateIf");
    }

    Value conditionValue = evaluateExpression(ifNode->condition.get());
    if (conditionValue.type != Value::Type::Bool) {
        throw std::runtime_error("Runtime error: condition is not a bool.");
        exit(3);
    }

    if (conditionValue.asBool()) {
        evaluateBlock(static_cast<const BlockNode*>(ifNode->trueBranch.get()));
    } else if (ifNode->falseBranch) {
        ASTNode* elseBranchNode = ifNode->falseBranch.get();
        if (elseBranchNode->getType() == ASTNode::Type::IfNode) {
            evaluateIf(elseBranchNode);
        } else {
            evaluateBlock(static_cast<const BlockNode*>(elseBranchNode));
        }
    }
}


// Evaluate the while node
void evaluateWhile(const WhileNode* node) {
    while (true) {
        Value condValue = evaluateExpression(node->condition.get());

        if (condValue.type != Value::Type::Bool) {
            throw std::runtime_error("Runtime error: condition is not a bool.");
            exit(3);
        }

        if (!condValue.asBool()) break;

        const BlockNode* blockNode = dynamic_cast<const BlockNode*>(node->body.get());
        if (!blockNode) {
            throw std::runtime_error("Non-block node passed to evaluateWhile");
        }
        evaluateBlock(blockNode);
    }
}


// Evaluate the print node
void evaluatePrint(const PrintNode* node) {
    Value value = evaluateExpression(node->expression.get());
    if (value.type == Value::Type::Double) {
        std::cout << value.asDouble() << std::endl;
    } else if (value.type == Value::Type::Bool) {
        std::cout << std::boolalpha << value.asBool() << std::endl;
    }
}

Value evaluateBinaryOperation(const BinaryOpNode* node) {
    Value left = evaluateExpression(node->left.get());
    Value right = evaluateExpression(node->right.get());

    if (left.type != right.type) {
        throw std::runtime_error("Type mismatch in binary operation.");
    }

    switch (node->op.type) {
        case TokenType::ADD:
            return Value(left.asDouble() + right.asDouble());
        case TokenType::SUBTRACT:
            return Value(left.asDouble() - right.asDouble());
        case TokenType::MULTIPLY:
            return Value(left.asDouble() * right.asDouble());
        case TokenType::DIVIDE:
            if (right.asDouble() == 0) {
                throw std::runtime_error("Division by zero.");
            }
            return Value(left.asDouble() / right.asDouble());
        case TokenType::MODULO:
            if (right.asDouble() == 0) {
                throw std::runtime_error("Modulo by zero.");
            }
            return Value(fmod(left.asDouble(), right.asDouble()));
        case TokenType::LESS:
            return Value(left.asDouble() < right.asDouble());
        case TokenType::LESS_EQUAL:
            return Value(left.asDouble() <= right.asDouble());
        case TokenType::GREATER:
            return Value(left.asDouble() > right.asDouble());
        case TokenType::GREATER_EQUAL:
            return Value(left.asDouble() >= right.asDouble());
        case TokenType::EQUAL:
            return Value(left.asDouble() == right.asDouble());
        case TokenType::NOT_EQUAL:
            return Value(left.asDouble() != right.asDouble());
        case TokenType::LOGICAL_AND:
            return Value(left.asBool() && right.asBool());
        case TokenType::LOGICAL_OR:
            return Value(left.asBool() || right.asBool());
        case TokenType::ASSIGN:
            throw std::runtime_error("Assignment should not be handled in evaluateBinaryOperation.");
        default:
            throw std::runtime_error("Unsupported binary operator.");
    }
}

// Function to evaluate variable access
Value evaluateVariable(const VariableNode* node) {
    auto iter = variables.find(node->identifier.value);
    if (iter != variables.end()) {
        return iter->second;
    } else {
        throw std::runtime_error("Variable not defined: " + node->identifier.value);
    }
}

Value evaluateAssignment(const AssignmentNode* assignmentNode) {
    if (!assignmentNode) {
        throw std::runtime_error("Null assignment node passed to evaluateAssignment");
    }

    Value value = evaluateExpression(assignmentNode->expression.get());
    variables[assignmentNode->identifier.value] = value;
    return value;
}

int main() {
    std::ostream& os = std::cout;
    std::string line;
    std::string inputCode;
    while (std::getline(std::cin, line)) {
        inputCode += line + "\n";
    }

    try {
        Lexer lexer(inputCode);
        auto tokens = lexer.tokenize();
        if (lexer.isSyntaxError(tokens)) {
            exit(1);
        }
        Parser parser(tokens);
        auto ast = parser.parse();

        if (ast->getType() == ASTNode::Type::BlockNode) {
            evaluateBlock(static_cast<const BlockNode*>(ast.get()));
        } else {
            throw std::runtime_error("");
        }

    } catch (const std::runtime_error& e) {
        os << e.what() << std::endl;
        exit(2);
    }
    return 0;
}
