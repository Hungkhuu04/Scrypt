#include "lib/ASTNodes.h"
#include "lib/mParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <sstream>

//  function to create an indentation string
std::string indentString(int indentLevel) {
    return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
}


// Formatter functions
std::string formatAST(const std::unique_ptr<ASTNode>& node, int indent = 0);
std::string formatBinaryOpNode(const BinaryOpNode* node, int indent) {
    std::stringstream ss;
    
    ss << '(' << formatAST(node->left, 0);  
    ss << ' ' << node->op.value << ' ';    
    ss << formatAST(node->right, 0) << ')'; 
    return ss.str();
}


std::string formatNumberNode(const NumberNode* node, int indent) {
    return indentString(indent) + node->value.value;
}

std::string formatBooleanNode(const BooleanNode* node, int indent) {
    return indentString(indent) + node->value.value;
}

std::string formatVariableNode(const VariableNode* node, int indent) {
    return indentString(indent) + node->identifier.value;
}

std::string formatIfNode(const IfNode* node, int indent) {
    std::stringstream ss;
    ss << indentString(indent) << "if ";
    ss << formatAST(node->condition, 0);
    ss << " {\n";
    ss << formatAST(node->trueBranch, indent + 1);
    if (node->falseBranch) {
        ss << "\n" << indentString(indent) << "} else {\n";
        ss << formatAST(node->falseBranch, indent + 1);
    }
    ss << "\n" << indentString(indent) << "}";
    return ss.str();
}

std::string formatAssignmentNode(const AssignmentNode* node, int indent) {
    std::stringstream ss;
    ss << indentString(indent) << '(' << node->identifier.value;
    ss << " = " << formatAST(node->expression, 0) << ')';
    return ss.str();
}


std::string formatWhileNode(const WhileNode* node, int indent) {
    std::stringstream ss;
    ss << indentString(indent) << "while " << formatAST(node->condition, 0) << " {\n";
    ss << formatAST(node->body, indent + 1);
    ss << "\n" << indentString(indent) << "}";
    return ss.str();
}

std::string formatPrintNode(const PrintNode* node, int indent) {
    std::stringstream ss;
    ss << indentString(indent) << "print " << formatAST(node->expression, 0);
    return ss.str();
}

std::string formatBlockNode(const BlockNode* node, int indent) {
    std::stringstream ss;
    for (const auto& stmt : node->statements) {
        ss << formatAST(stmt, indent) << "\n";
    }
    std::string blockString = ss.str();
    if (!node->statements.empty()) {
        blockString.pop_back();
    }
    return blockString;
}

// Formatting function for each AST node type
std::string formatAST(const std::unique_ptr<ASTNode>& node, int indent) {
    if (!node) return "";

    switch (node->getType()) {
        case ASTNode::Type::BinaryOpNode:
            return formatBinaryOpNode(static_cast<const BinaryOpNode*>(node.get()), indent);
        case ASTNode::Type::NumberNode:
            return formatNumberNode(static_cast<const NumberNode*>(node.get()), indent);
        case ASTNode::Type::BooleanNode:
            return formatBooleanNode(static_cast<const BooleanNode*>(node.get()), indent);
        case ASTNode::Type::VariableNode:
            return formatVariableNode(static_cast<const VariableNode*>(node.get()), indent);
        case ASTNode::Type::AssignmentNode:
            return formatAssignmentNode(static_cast<const AssignmentNode*>(node.get()), indent);
        case ASTNode::Type::PrintNode:
            return formatPrintNode(static_cast<const PrintNode*>(node.get()), indent);
        case ASTNode::Type::IfNode:
            return formatIfNode(static_cast<const IfNode*>(node.get()), indent);
        case ASTNode::Type::WhileNode:
            return formatWhileNode(static_cast<const WhileNode*>(node.get()), indent);
        case ASTNode::Type::BlockNode:
            return formatBlockNode(static_cast<const BlockNode*>(node.get()), indent);
        default:
            return "/* Unknown node type */";
    }
}



// Code to handle input, parse it into an AST, and then call formatAST on the root...
// This loop will continue until EOF (Ctrl+D or Ctrl+Z followed by Enter)
int main() {
    
    std::string line;
    std::string inputCode;
    while (std::getline(std::cin, line)) {
        inputCode += line + "\n";
    }

    Lexer lexer(inputCode);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<ASTNode> ast = parser.parse();

    std::string formattedCode = formatAST(ast);

    std::cout << formattedCode << std::endl;

    return 0;
}
