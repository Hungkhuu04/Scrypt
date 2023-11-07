#include "lib/lex.h"
#include "lib/mParser.h"
#include <iostream>
#include <string>
using namespace std;

std::string tokenValue(const mNode* node) {
    switch (node->type) {
        case mNodeType::NUMBER:
            return std::to_string(node->value);
        case mNodeType::IDENTIFIER:
            return node->identifier;
        case mNodeType::ADD:
            return "+";
        case mNodeType::SUBTRACT:
            return "-";
        case mNodeType::MULTIPLY:
            return "*";
        case mNodeType::DIVIDE:
            return "/";
        case mNodeType::MODULO:
            return "%";
        case mNodeType::LESS:
            return "<";
        case mNodeType::LESS_EQUAL:
            return "<=";
        case mNodeType::GREATER_THAN:
            return ">";
        case mNodeType::GREATER_EQUAL:
            return ">=";
        case mNodeType::EQUAL:
            return "==";
        case mNodeType::NOT_EQUAL:
            return "!=";
        case mNodeType::LOGICAL_AND:
            return "&";
        case mNodeType::LOGICAL_XOR:
            return "^";
        case mNodeType::LOGICAL_OR:
            return "|";
        case mNodeType::ASSIGN:
            return "=";
        // Add additional cases for other node types as needed
        default:
            return "unknown";
    }
}
void formatAST(mNode* node, std::ostream& os, int depth = 0) {
    if (!node) return;

    // Indentation for nested structures
    std::string indent(depth * 4, ' ');

    switch (node->type) {
        case mNodeType::ASSIGN:
        case mNodeType::ADD:
        case mNodeType::SUBTRACT:
        case mNodeType::MULTIPLY:
        case mNodeType::DIVIDE:
        case mNodeType::MODULO:
        case mNodeType::LESS:
        case mNodeType::LESS_EQUAL:
        case mNodeType::GREATER_THAN:
        case mNodeType::GREATER_EQUAL:
        case mNodeType::EQUAL:
        case mNodeType::NOT_EQUAL:
        case mNodeType::LOGICAL_AND:
        case mNodeType::LOGICAL_OR:
            os << indent << "(";
            formatAST(node->children[0], os, depth + 1);
            os << " " << tokenValue(node) << " ";
            formatAST(node->children[1], os, depth + 1);
            os << ")";
            break;
        case mNodeType::IF_STATEMENT:
            os << indent << "if (";
            formatAST(node->condition, os, depth + 1);
            os << ") {\n";
            formatAST(node->thenBranch, os, depth + 1);
            os << indent << "}";
            if (node->elseBranch) {
                os << " else {\n";
                formatAST(node->elseBranch, os, depth + 1);
                os << indent << "}";
            }
            break;
        case mNodeType::WHILE_STATEMENT:
            os << indent << "while (";
            formatAST(node->condition, os, depth + 1);
            os << ") {\n";
            formatAST(node->body, os, depth + 1);
            os << indent << "}";
            break;
        case mNodeType::PRINT_STATEMENT:
            os << indent << "print ";
            formatAST(node->Lvalue, os, depth);
            break;
        case mNodeType::NUMBER:
        case mNodeType::IDENTIFIER:
        case mNodeType::BOOLEAN_LITERAL:
            os << indent <<tokenValue(node);
            break;
        case mNodeType::BLOCK:
            for (mNode* stmt : node->statements) {
                formatAST(stmt, os, depth + 1);
                os << "\n";
            }
            break;
        default:
            // Handle other types or errors
            break;
    }
}



int main() {
    std::string code;
    std::string line;
    std::cout << "Enter your code (Ctrl+D to finish):\n";
    while (std::getline(std::cin, line)) {
        code += line + '\n'; // Add the line and a newline character to code
    }

    // Here you need to tokenize the input. 
    // Assuming you have a function called `lex` which does this.
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    // Create a parser instance with the tokens.
    mParser parser(tokens);
    
    // Parse the tokens into an AST.
    mNode* ast = nullptr;
    try {
        ast = parser.parse(std::cerr);
        // Format and print the AST.
        formatAST(ast, std::cout);
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // Clean up the AST to avoid memory leaks.
    parser.clearTree(ast);

    return 0;
}