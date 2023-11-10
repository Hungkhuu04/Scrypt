#include "lib/ASTNodes.h"
#include "lib/mParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <ostream>
#include <cmath>
#include <iomanip>

// function to create an indentation string
std::string indentString(int indentLevel) {
    return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
}

// function to format the AST
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent = 0);

// function to format operation types
void formatBinaryOpNode(std::ostream& os, const BinaryOpNode* node, int indent) {
    os << '(';
    formatAST(os, node->left, 0);  
    os << ' ' << node->op.value << ' ';    
    formatAST(os, node->right, 0);
    os << ')';
}

// function to format numbers (especially doubles)
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

// function to format Booleans
void formatBooleanNode(std::ostream& os, const BooleanNode* node, int indent) {
    os << indentString(indent) << node->value.value;
}

// function to format Variables
void formatVariableNode(std::ostream& os, const VariableNode* node, int indent) {
    os << indentString(indent) << node->identifier.value;
}

// function to format if nodes
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

// function to format assignment nodes
void formatAssignmentNode(std::ostream& os, const AssignmentNode* node, int indent) {
    os << indentString(indent) << "(" << node->identifier.value;
    os << " = ";
    formatAST(os, node->expression, 0);
    os << ")";
}

// function to format while nodes
void formatWhileNode(std::ostream& os, const WhileNode* node, int indent) {
    os << indentString(indent) << "while ";
    formatAST(os, node->condition, 0);
    os << " {\n";
    formatAST(os, node->body, indent + 1);
    os << "\n" << indentString(indent) << "}";
}

// function to format print nodes
void formatPrintNode(std::ostream& os, const PrintNode* node, int indent) {
    os << indentString(indent) << "print ";
    formatAST(os, node->expression, 0);
}

// function to format block nodes
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

// main format function
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent) {
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
        std::unique_ptr<ASTNode> ast;
        ast = parser.parse();
        formatAST(std::cout, ast);
        os << std::endl;
    } catch (const std::runtime_error& e) {
        os << e.what() << std::endl;
        exit(2);
    }
    return 0;
}
