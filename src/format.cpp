#include "lib/lex.h"
#include "lib/infixParser.h"
#include <iostream>
#include <string>
using namespace std;
std::string formatDecimal(double value) {
    if (value == static_cast<int>(value)) {
        return to_string(static_cast<int>(value));
    } else {
        ostringstream ss;
        ss << value;
        return ss.str();
    }
}

std::string format(Node* node, std::ostream& os = std::cout) {
    if (!node) {
        return "";
    }
    
    std::string result;
    
    switch (node->type) {
        case NodeType::NUMBER:
            result = formatDecimal(node->value);  // assuming formatDecimal is defined elsewhere
            break;
        case NodeType::ADD:
            result = "(" + format(node->children[0], os) + " + " + format(node->children[1], os) + ")";
            break;
        case NodeType::SUBTRACT:
            result = "(" + format(node->children[0], os) + " - " + format(node->children[1], os) + ")";
            break;
        case NodeType::MULTIPLY:
            result = "(" + format(node->children[0], os) + " * " + format(node->children[1], os) + ")";
            break;
        case NodeType::DIVIDE:
            result = "(" + format(node->children[0], os) + " / " + format(node->children[1], os) + ")";
            break;
        case NodeType::ASSIGN:
            result = "(" + node->children[0]->identifier + " = " + format(node->children[1], os) + ")";
            break;
        case NodeType::IDENTIFIER:
            result = node->identifier;
            break;
        case NodeType::BOOLEAN_LITERAL:
            result = (node->value == 1) ? "true" : "false";
            break;
        // New node types for logical and relational operators
        case NodeType::LESS_THAN:
            result = "(" + format(node->children[0], os) + " < " + format(node->children[1], os) + ")";
            break;
        case NodeType::LESS_EQUAL:
            result = "(" + format(node->children[0], os) + " <= " + format(node->children[1], os) + ")";
            break;
        case NodeType::GREATER_THAN:
            result = "(" + format(node->children[0], os) + " > " + format(node->children[1], os) + ")";
            break;
        case NodeType::GREATER_EQUAL:
            result = "(" + format(node->children[0], os) + " >= " + format(node->children[1], os) + ")";
            break;
        case NodeType::EQUAL:
            result = "(" + format(node->children[0], os) + " == " + format(node->children[1], os) + ")";
            break;
        case NodeType::NOT_EQUAL:
            result = "(" + format(node->children[0], os) + " != " + format(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_AND:
            result = "(" + format(node->children[0], os) + " & " + format(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_OR:
            result = "(" + format(node->children[0], os) + " | " + format(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_XOR:
            result = "(" + format(node->children[0], os) + " ^ " + format(node->children[1], os) + ")";
            break;
        case NodeType::MODULO:
            return "(" + format(node->children[0]) + " % " + format(node->children[1]) + ")";
        default:
            os << "Error: Unknown node type encountered while constructing infix string.\n";
            exit(1);
    }
    return result;
}

void clearTree(Node* node) {
    if (node == nullptr) return;

    // Recursively delete children nodes
    for (auto& child : node->children) {
        clearTree(child);
    }

    // Delete the current node
    delete node;
}


int main() {
    std::ostream& os = std::cout;
    std::vector<std::string> expressions;
    std::string line;

    // Read lines until EOF is encountered (Ctrl+D)
    while (std::getline(std::cin, line)) {
        // Skip empty lines
        if (!line.empty()) {
            expressions.push_back(line);
        }
    }

    std::vector<Node*> statements;
    for (const auto& expr : expressions) {
        Lexer lexer(expr);
        auto tokens = lexer.tokenize();

        if (lexer.isSyntaxError(tokens)) {
            throw std::runtime_error("");
        }

        InfixParser parser(tokens);
        Node* parsedExpr = nullptr;
        try {
            parsedExpr = parser.parse(os);
        } catch (const std::runtime_error& e) {
            os << "Parse error: " << e.what() << std::endl;
            continue;
        }

        if (parsedExpr) {
            statements.push_back(parsedExpr);
        }
    }

    // Output formatted blocks of code
    for (auto& statement : statements) {
        os << format(statement) << std::endl;
    }

    // Clean up the AST to prevent memory leaks
    for (auto& statement : statements) {
        clearTree(statement);
    }

    return 0;
}
