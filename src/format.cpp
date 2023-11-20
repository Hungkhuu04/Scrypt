
#include "lib/ASTNodes.h"
#include "lib/mParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <ostream>
#include <cmath>
#include <iomanip>

std::string indentString(int indentLevel);
void formatAST(std::ostream& os, const std::unique_ptr<ASTNode>& node, int indent = 0);
void formatBinaryOpNode(std::ostream& os, const BinaryOpNode* node, int indent);
void formatNumberNode(std::ostream& os, const NumberNode* node, int indent);
void formatBooleanNode(std::ostream& os, const BooleanNode* node, int indent);
void formatVariableNode(std::ostream& os, const VariableNode* node, int indent);
void formatIfNode(std::ostream& os, const IfNode* node, int indent);
void formatAssignmentNode(std::ostream& os, const AssignmentNode* node, int indent);
void formatWhileNode(std::ostream& os, const WhileNode* node, int indent);
void formatPrintNode(std::ostream& os, const PrintNode* node, int indent);
void formatBlockNode(std::ostream& os, const BlockNode* node, int indent);
void formatFunctionNode(std::ostream& os, const FunctionNode* node, int indent);
void formatReturnNode(std::ostream& os, const ReturnNode* node, int indent);
void formatCallNode(std::ostream& os, const CallNode* node, int indent);
void formatNullNode(std::ostream& os, const NullNode* node, int indent);

// function to create an indentation string
std::string indentString(int indentLevel) {
    return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
}

void formatNullNode(std::ostream& os, const NullNode* node, int indent) {
    os << indentString(indent) << "null";
}

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
    os << ");"; // Add a semicolon here
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
    os << ";"; // Add a semicolon here
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
        case ASTNode::Type::FunctionNode:
            formatFunctionNode(os, static_cast<const FunctionNode*>(node.get()), indent);
            break;
        case ASTNode::Type::ReturnNode:
            formatReturnNode(os, static_cast<const ReturnNode*>(node.get()), indent);
            break;
        case ASTNode::Type::CallNode:
            formatCallNode(os, static_cast<const CallNode*>(node.get()), indent);
            break;
        case ASTNode::Type::NullNode:
            formatNullNode(os, static_cast<const NullNode*>(node.get()), indent);
            break;
        default:
            os << indentString(indent) << "/* Unknown node type */";
            break;
    }
}


// Function to format FunctionNode (function definitions)
void formatFunctionNode(std::ostream& os, const FunctionNode* node, int indent) {
    os << indentString(indent) << "def " << node->name.value << "(";
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        os << node->parameters[i].value;
        if (i < node->parameters.size() - 1) {
            os << ", ";
        }
    }
    os << ") {";
    
    const BlockNode* blockNode = dynamic_cast<const BlockNode*>(node->body.get());
    if (blockNode && !blockNode->statements.empty()) {
        os << "\n";
        formatAST(os, node->body, indent + 1);
        os << "\n" << indentString(indent);
    } else {
        os << "\n" << indentString(indent); // Add newline and indentation for empty body
    }
    os << "}"; // Closing brace
}




// Function to format ReturnNode (return statements)
void formatReturnNode(std::ostream& os, const ReturnNode* node, int indent) {
    os << indentString(indent) << "return";
    if (node->value) {
        os << " ";
        formatAST(os, node->value, 0);
    }
    os << ";"; // Ensure semicolon is outside of the conditional
}

// Function to format CallNode (function calls)

void formatCallNode(std::ostream& os, const CallNode* node, int indent, bool isStandalone = true) {
    formatAST(os, node->callee, indent);
    os << '(';
    for (size_t i = 0; i < node->arguments.size(); ++i) {
        formatAST(os, node->arguments[i], 0);
        if (i < node->arguments.size() - 1) {
            os << ", ";
        }
    }
    os << ')';
    if (isStandalone) {
        os << ";"; // Add a semicolon if it's a standalone function call
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
    } catch (...){
        os << "Unknown error" << std::endl;
        exit(2);
    }
    return 0;
}