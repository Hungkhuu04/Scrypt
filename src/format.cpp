#include "lib/lex.h"
#include "lib/infixParser.h"
#include <iostream>
#include <string>

std::string formatNode(Node* node) {
    if (node == nullptr) return "";
    std::string formatted;

    switch (node->type) {
        case NodeType::ASSIGN:
            if (node->children.size() >= 2) { // Make sure there are at least two children
                formatted += "(" + formatNode(node->children[0]) + " = " + formatNode(node->children[1]) + ")";
            }
            break;
        case NodeType::ADD:
        case NodeType::SUBTRACT:
        case NodeType::MULTIPLY:
        case NodeType::DIVIDE:
        case NodeType::MODULO:
        case NodeType::LESS_THAN:
        case NodeType::LESS_EQUAL:
        case NodeType::GREATER_THAN:
        case NodeType::GREATER_EQUAL:
        case NodeType::EQUAL:
        case NodeType::NOT_EQUAL:
        case NodeType::LOGICAL_AND:
        case NodeType::LOGICAL_XOR:
        case NodeType::LOGICAL_OR:
            if (node->children.size() >= 2) { // Binary operations require two children
                const char* op = "";
                switch (node->type) {
                    case NodeType::ADD: op = " + "; break;
                    case NodeType::SUBTRACT: op = " - "; break;
                    case NodeType::MULTIPLY: op = " * "; break;
                    case NodeType::DIVIDE: op = " / "; break;
                    case NodeType::MODULO: op = " % "; break;
                    case NodeType::LESS_THAN: op = " < "; break;
                    case NodeType::LESS_EQUAL: op = " <= "; break;
                    case NodeType::GREATER_THAN: op = " > "; break;
                    case NodeType::GREATER_EQUAL: op = " >= "; break;
                    case NodeType::EQUAL: op = " == "; break;
                    case NodeType::NOT_EQUAL: op = " != "; break;
                    case NodeType::LOGICAL_AND: op = " & "; break;
                    case NodeType::LOGICAL_XOR: op = " ^ "; break;
                    case NodeType::LOGICAL_OR: op = " | "; break;
                    default: break;
                }
                formatted += "(" + formatNode(node->children[0]) + op + formatNode(node->children[1]) + ")";
            }
            break;
        case NodeType::NUMBER:
            formatted += std::to_string(node->value);
            break;
        case NodeType::IDENTIFIER:
            formatted += node->identifier;
            break;
        case NodeType::BOOLEAN_LITERAL:
            formatted += (node->boolValue ? "true" : "false");
            break;
        case NodeType::IF_STATEMENT:
            formatted += "if (" + formatNode(node->condition) + ") {\n";
            formatted += formatNode(node->thenBranch);
            if (node->elseBranch != nullptr) {
                formatted += "}\nelse {\n";
                formatted += formatNode(node->elseBranch);
            }
            formatted += "}\n";
            break;
            
        case NodeType::WHILE_STATEMENT:
            formatted += "while (" + formatNode(node->condition) + ") {\n";
            for (auto& child : node->body) {
                formatted += formatNode(child) + "\n";
            }
            formatted += "}\n";
            break;
            
        case NodeType::PRINT_STATEMENT:
            formatted += "print " + formatNode(node->children[0]);
            break;
            
        case NodeType::BLOCK:
            for (auto& child : node->children) {
                formatted += formatNode(child) + "\n";
            }
            break;
            
        case NodeType::STATEMENT:
            for (auto& child : node->children) {
                formatted += "(" + formatNode(child) + ")";
            }
            break;
        default:
            // Handle other complex structures
            // ...
            break;
    }

    return formatted;
}

// Helper function to format a statement node
std::string formatStatement(Node* node) {
    return formatNode(node) + ";\n"; // Assuming each statement ends with a semicolon
}

// Helper function to format a block of statements
std::string formatBlock(const std::vector<Node*>& statements) {
    std::string formatted;
    for (const auto& statement : statements) {
        formatted += "    " + formatNode(statement) + "\n"; // Added newline for formatting
    }
    return formatted;
}

int main() {
    std::string code;
    std::string line;
    std::ostream& os = std::cout; // You can replace std::cout with another ostream if needed

    os << "Enter code (press Ctrl+D on a new line to finish):" << std::endl;

    // Read lines until EOF is encountered (Ctrl+D)
    while (std::getline(std::cin, line)) {
        code += line + "\n";
    }

    if (code.empty()) {
        os << "No input provided." << std::endl;
        return 1;
    }

    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    if (lexer.isSyntaxError(tokens)) {
        for (const auto& error : lexer.errors) {
            os << "Syntax error: " << error << std::endl;
        }
        return 1;
    }

    InfixParser parser(tokens);
    Node* root = parser.parse(os); // Now parse the tokens

    if (!root) {
        os << "Parse error: The input did not result in a valid syntax tree." << std::endl;
        return 1;
    }

    // Check if the root is a BLOCK node and format it accordingly
    if (root->type == NodeType::BLOCK) {
        std::string formattedCode = formatBlock(root->children);
        os << formattedCode;
    } else {
        // If the root is not a BLOCK, you might want to handle this case differently.
        // For now, let's just format whatever the root node is.
        os << formatNode(root);
    }
    os << std::endl;

    // Don't forget to clean up the AST to prevent memory leaks
    parser.clearTree(root);

    return 0;
}