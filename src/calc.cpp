#include "lib/mParser.h"
#include "lib/ASTNodes.h" 
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include "lib/ScryptComponents.h"
#include <iomanip>
#include <cmath>

using namespace std;

std::unordered_map<std::string, Value> variables;
void formatAST(std::ostream &os, const std::unique_ptr<ASTNode> &node, int indent = 0);


// Function to create an indentation string
std::string indentString(int indentLevel) {
    return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
}

// Format Binary Operation Node
void formatBinaryOpNode(std::ostream& os, const BinaryOpNode* node, int indent) {
    os << '(';
    formatAST(os, node->left, 0);  
    os << ' ' << node->op.value << ' ';    
    formatAST(os, node->right, 0);
    os << ')';
}

// Format Number Node
void formatNumberNode(std::ostream& os, const NumberNode* node, int indent) {
    double value = std::stod(node->value.value);
    if (std::floor(value) == value) {
        os << indentString(indent) << static_cast<long>(value);
    } else {
        std::ostringstream tempStream;
        tempStream << std::fixed << std::setprecision(2) << value;
        std::string str = tempStream.str();
        str.erase(str.find_last_not_of('0') + 1, std::string::npos); 
        if (str.back() == '.') {
            str.pop_back(); 
        }
        os << indentString(indent) << str;
    }
}

// Format Boolean Node
void formatBooleanNode(std::ostream& os, const BooleanNode* node, int indent) {
    os << indentString(indent) << node->value.value;
}

// Format Variable Node
void formatVariableNode(std::ostream& os, const VariableNode* node, int indent) {
    os << indentString(indent) << node->identifier.value;
}

// Format If Node
void formatIfNode(std::ostream& os, const IfNode* node, int indent) {
    os << indentString(indent) << "if ";
    formatAST(os, node->condition, 0);
    os << " {\n";
    formatAST(os, node->trueBranch, indent + 1);
    if (node->falseBranch) {
        os << "\n" << indentString(indent) << "}\n" << indentString(indent) << "else {\n";
        formatAST(os, node->falseBranch, indent + 1);
    }
    os << "\n" << indentString(indent) << "}";
}

void formatArrayLookupNode(std::ostream& os, const ArrayLookupNode* node, int indent) {
    formatAST(os, node->array, indent);
    os << "[";
    formatAST(os, node->index, 0);
    os << "]";
}

// Format Assignment Node
void formatAssignmentNode(std::ostream& os, const AssignmentNode* node, int indent) {
    os << indentString(indent);

    // Handle both VariableNode and ArrayLookupNode on left-hand side
    if (node->lhs->getType() == ASTNode::Type::VariableNode) {
        auto variableNode = static_cast<const VariableNode*>(node->lhs.get());
        os << variableNode->identifier.value << " = ";
    } else if (node->lhs->getType() == ASTNode::Type::ArrayLookupNode) {
        auto arrayLookupNode = static_cast<const ArrayLookupNode*>(node->lhs.get());
        formatArrayLookupNode(os, arrayLookupNode, 0);
        os << " = ";
    } else {
        throw std::runtime_error("Invalid left-hand side in assignment");
    }

    formatAST(os, node->rhs, 0);
    os << ";";
}

// Format While Node
void formatWhileNode(std::ostream& os, const WhileNode* node, int indent) {
    os << indentString(indent) << "while ";
    formatAST(os, node->condition, 0);
    os << " {\n";
    formatAST(os, node->body, indent + 1);
    os << "\n" << indentString(indent) << "}";
}

// Format Print Node
void formatPrintNode(std::ostream& os, const PrintNode* node, int indent) {
    os << indentString(indent) << "print ";
    formatAST(os, node->expression, 0);
}

// Format Block Node
void formatBlockNode(std::ostream& os, const BlockNode* node, int indent) {
    bool isFirstStatement = true;
    for (const auto& stmt : node->statements) {
        if (!isFirstStatement) {
            os << "\n";
        }
        formatAST(os, stmt, indent);
        isFirstStatement = false;
    }
}

void formatAST(std::ostream &os, const std::unique_ptr<ASTNode> &node, int indent = 0){
    if (!node) return;

    switch (node->getType()) {
        case ASTNode::Type::BinaryOpNode:
            formatBinaryOpNode(os, static_cast<const BinaryOpNode*>(node.get()), indent);
            break;
        case ASTNode::Type::NumberNode:
            formatNumberNode(os, static_cast<const NumberNode*>(node.get()), indent);
            break;
        case ASTNode::Type::BooleanNode:
            formatBooleanNode(os, static_cast<const BooleanNode*>(node.get()), indent);
            break;
        case ASTNode::Type::VariableNode:
            formatVariableNode(os, static_cast<const VariableNode*>(node.get()), indent);
            break;
        case ASTNode::Type::AssignmentNode:
            formatAssignmentNode(os, static_cast<const AssignmentNode*>(node.get()), indent);
            break;
        case ASTNode::Type::PrintNode:
            formatPrintNode(os, static_cast<const PrintNode*>(node.get()), indent);
            break;
        case ASTNode::Type::IfNode:
            formatIfNode(os, static_cast<const IfNode*>(node.get()), indent);
            break;
        case ASTNode::Type::WhileNode:
            formatWhileNode(os, static_cast<const WhileNode*>(node.get()), indent);
            break;
        case ASTNode::Type::BlockNode:
            formatBlockNode(os, static_cast<const BlockNode*>(node.get()), indent);
            break;
        default:
            os << indentString(indent) << "/* Unknown node type */";
            break;
    }
}


Value evaluate(const std::unique_ptr<ASTNode>& node) {
    if (!node) return Value();

    switch (node->getType()) {
        case ASTNode::Type::NumberNode: {
            const auto* numNode = static_cast<const NumberNode*>(node.get());
            return Value(std::stod(numNode->value.value));
        }

        case ASTNode::Type::BooleanNode: {
            const auto* boolNode = static_cast<const BooleanNode*>(node.get());
            return Value(boolNode->value.value == "true");
        }

        case ASTNode::Type::BinaryOpNode: {
            const auto* binNode = static_cast<const BinaryOpNode*>(node.get());
            Value left = evaluate(binNode->left);
            Value right = evaluate(binNode->right);
            break;
        }

        case ASTNode::Type::VariableNode: {
            const auto* varNode = static_cast<const VariableNode*>(node.get());
            if (variables.find(varNode->identifier.value) != variables.end()) {
                return variables[varNode->identifier.value];
            }
            throw std::runtime_error("Variable not found: " + varNode->identifier.value);
        }

        case ASTNode::Type::AssignmentNode: {
            const auto* assignNode = static_cast<const AssignmentNode*>(node.get());
            Value rhsValue = evaluate(assignNode->rhs);

            if (assignNode->lhs->getType() == ASTNode::Type::VariableNode) {
                auto varNode = static_cast<const VariableNode*>(assignNode->lhs.get());
                variables[varNode->identifier.value] = rhsValue;
            } else if (assignNode->lhs->getType() == ASTNode::Type::ArrayLookupNode) {
            } else {
                throw std::runtime_error("Invalid left-hand side in assignment");
            }

            return rhsValue;
        }


        case ASTNode::Type::IfNode: {
            const auto* ifNode = static_cast<const IfNode*>(node.get());
            Value condition = evaluate(ifNode->condition);
            bool conditionAsBool = false;
            if (conditionAsBool) {
                return evaluate(ifNode->trueBranch);
            } else if (ifNode->falseBranch) {
                return evaluate(ifNode->falseBranch);
            }
            return Value();
        }

        case ASTNode::Type::PrintNode: {
            const auto* printNode = static_cast<const PrintNode*>(node.get());
            Value value = evaluate(printNode->expression);
            std::string valueAsString;
            std::cout << valueAsString << std::endl;
            return Value();
        }

        case ASTNode::Type::WhileNode: {
            const auto* whileNode = static_cast<const WhileNode*>(node.get());
            while (true) {
                Value condition = evaluate(whileNode->condition);
                bool conditionAsBool = false; // Convert condition to boolean
                if (!conditionAsBool) break;
                evaluate(whileNode->body);
            }
            return Value();
        }

        case ASTNode::Type::FunctionNode: {
            const auto* funcNode = static_cast<const FunctionNode*>(node.get());
            return Value(); // Function definitions don't return a value
        }

        default:
            throw std::runtime_error("Unknown node type in evaluation");
    }
}





int main() {
    std::ostream& os = std::cout;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }

        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();

            Parser parser(tokens);
            std::unique_ptr<ASTNode> ast = parser.parse();

            if (ast) {
                formatAST(os, ast);

                Value result = evaluate(ast);
                std::string resultString;


                os << " = " << resultString << std::endl;  
            } else {
                os << "No AST generated." << std::endl;
            }
        } catch (const std::runtime_error& e) {
            os << "Runtime error: " << e.what() << std::endl;
        } catch (...) {
            os << "Unknown error" << std::endl;
        }
    }

    return 0;
}