#include "mParser.h"
#include "Lexer.h" // Custom lexer header file for tokenizing input
#include "ASTNodes.h" // Contains definitions for all ASTNode types
#include <iostream>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, Value> variables;

// Format and evaluate functions declaration
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent = 0);
Value evaluate(const std::unique_ptr<ASTNode>& node);

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }

        try {
            // Tokenize the input
            Lexer lexer(line);
            auto tokens = lexer.tokenize();

            // Parse the tokens into an AST
            Parser parser(tokens);
            std::unique_ptr<ASTNode> ast = parser.parse();

            // Evaluate the AST
            Value result = evaluate(ast);

            // Format and output the AST (optional, for debugging or visualization)
            formatAST(std::cout, ast);
            std::cout << "Result: " << result.toString() << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}

// Functions to format and evaluate the AST (implementation needed)

void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent = 0) {
    if (!node) return;

    std::string indention = std::string(indent * 4, ' '); // 4 spaces per indent level

    switch (node->getType()) {
        case ASTNode::Type::BinaryOpNode: {
            auto binNode = static_cast<const BinaryOpNode*>(node.get());
            os << indention << "(" << binNode->op.value;
            formatAST(os, binNode->left, indent + 1);
            formatAST(os, binNode->right, indent + 1);
            os << ")";
            break;
        }
        case ASTNode::Type::NumberNode: {
            auto numNode = static_cast<const NumberNode*>(node.get());
            os << indention << numNode->value.value;
            break;
        }
        case ASTNode::Type::BooleanNode: {
            auto boolNode = static_cast<const BooleanNode*>(node.get());
            os << indention << (boolNode->value.value ? "true" : "false");
            break;
        }
        case ASTNode::Type::VariableNode: {
            auto varNode = static_cast<const VariableNode*>(node.get());
            os << indention << varNode->identifier.value;
            break;
        }
        case ASTNode::Type::AssignmentNode: {
            auto assignNode = static_cast<const AssignmentNode*>(node.get());
            os << indention << "(" << assignNode->identifier.value << " = ";
            formatAST(os, assignNode->expression, indent + 1);
            os << ")";
            break;
        }
        case ASTNode::Type::PrintNode: {
            auto printNode = static_cast<const PrintNode*>(node.get());
            os << indention << "print ";
            formatAST(os, printNode->expression, indent + 1);
            break;
        }
        case ASTNode::Type::IfNode: {
            auto ifNode = static_cast<const IfNode*>(node.get());
            os << indention << "if ";
            formatAST(os, ifNode->condition, indent + 1);
            os << " {\n";
            formatAST(os, ifNode->trueBranch, indent + 1);
            if (ifNode->falseBranch) {
                os << "\n" << indention << "} else {\n";
                formatAST(os, ifNode->falseBranch, indent + 1);
            }
            os << "\n" << indention << "}";
            break;
        }
        case ASTNode::Type::WhileNode: {
            auto whileNode = static_cast<const WhileNode*>(node.get());
            os << indention << "while ";
            formatAST(os, whileNode->condition, indent + 1);
            os << " {\n";
            formatAST(os, whileNode->body, indent + 1);
            os << "\n" << indention << "}";
            break;
        }
        case ASTNode::Type::BlockNode: {
            auto blockNode = static_cast<const BlockNode*>(node.get());
            for (const auto& stmt : blockNode->statements) {
                formatAST(os, stmt, indent + 1);
                os << "\n";
            }
            break;
        }
        default:
            os << indention << "/* Unknown node type */";
            break;
    }
}




Value evaluate(const std::unique_ptr<ASTNode>& node) {
    if (!node) {
        return Value();  // Handle null node
    }

    switch (node->getType()) {
        case ASTNode::Type::NumberNode: {
            auto numNode = static_cast<const NumberNode*>(node.get());
            return Value(std::stod(numNode->value.value));
        }

        case ASTNode::Type::BooleanNode: {
            auto boolNode = static_cast<const BooleanNode*>(node.get());
            return Value(boolNode->value.value == "true");
        }

        case ASTNode::Type::BinaryOpNode: {
            const auto* binNode = static_cast<const BinaryOpNode*>(node.get());
            Value left = evaluate(binNode->left);
            Value right = evaluate(binNode->right);

            switch (binNode->op.type) {
                case TokenType::ADD:
                    return left + right;
                case TokenType::SUBTRACT:
                    return left - right;
                case TokenType::MULTIPLY:
                    return left * right;
                case TokenType::DIVIDE:
                    if (right.isZero()) {
                        throw std::runtime_error("Division by zero");
                    }
                    return left / right;
                case TokenType::MODULO:
                    if (right.isZero()) {
                        throw std::runtime_error("Modulo by zero");
                    }
                    return left % right;
                case TokenType::LESS:
                    return left < right;
                case TokenType::LESS_EQUAL:
                    return left <= right;
                case TokenType::GREATER:
                    return left > right;
                case TokenType::GREATER_EQUAL:
                    return left >= right;
                case TokenType::EQUAL:
                    return left == right;
                case TokenType::NOT_EQUAL:
                    return left != right;
                case TokenType::LOGICAL_AND:
                    return left && right;
                case TokenType::LOGICAL_OR:
                    return left || right;
                case TokenType::LOGICAL_XOR:
                    return (left && !right) || (!left && right);
                default:
                    throw std::runtime_error("Unsupported binary operation");
            }
        }

        case ASTNode::Type::VariableNode: {
            const auto* varNode = static_cast<const VariableNode*>(node.get());
            auto varIt = variables.find(varNode->identifier.value);
            if (varIt == variables.end()) {
                throw std::runtime_error("Variable not found: " + varNode->identifier.value);
            }
            return varIt->second;
        }

        case ASTNode::Type::AssignmentNode: {
            const auto* assignNode = static_cast<const AssignmentNode*>(node.get());
            if (variables.find(assignNode->identifier.value) == variables.end()) {
                throw std::runtime_error("Invalid assignee: " + assignNode->identifier.value);
            }
            Value value = evaluate(assignNode->expression);
            variables[assignNode->identifier.value] = value;
            return value;
        }

        case ASTNode::Type::PrintNode: {
            const auto* printNode = static_cast<const PrintNode*>(node.get());
            Value value = evaluate(printNode->expression);
            std::cout << value.toString() << std::endl;
            return value;
        }

        case ASTNode::Type::IfNode: {
            const auto* ifNode = static_cast<const IfNode*>(node.get());
            Value condition = evaluate(ifNode->condition);
            if (condition.isTrue()) {
                return evaluate(ifNode->trueBranch);
            } else if (ifNode->falseBranch) {
                return evaluate(ifNode->falseBranch);
            }
            return Value();
        }

        case ASTNode::Type::WhileNode: {
            const auto* whileNode = static_cast<const WhileNode*>(node.get());
            while (evaluate(whileNode->condition).isTrue()) {
                evaluate(whileNode->body);
            }
            return Value();
        }

        case ASTNode::Type::BlockNode: {
            const auto* blockNode = static_cast<const BlockNode*>(node.get());
            Value lastValue;
            for (const auto& stmt : blockNode->statements) {
                lastValue = evaluate(stmt);
            }
            return lastValue;
        }

        default:
            throw std::runtime_error("Unknown node type in evaluation");
    }
}
