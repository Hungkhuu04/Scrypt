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

std::string format(Node* node, int indentLevel = 0 ,std::ostream& os = std::cout){
    if (!node) {
        return "";
    }
    
    std::string result;
    std::string indent(indentLevel, ' ');
    
    switch (node->type) {
        case NodeType::NUMBER:
            result = formatDecimal(node->value);  // assuming formatDecimal is defined elsewhere
            break;
        case NodeType::ADD:
            result = "(" + format(node->children[0], indentLevel, os) + " + " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::SUBTRACT:
            result = "(" + format(node->children[0], indentLevel, os) + " - " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::MULTIPLY:
            result = "(" + format(node->children[0], indentLevel, os) + " * " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::DIVIDE:
            result = "(" + format(node->children[0], indentLevel, os) + " / " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::ASSIGN:
            result = "(" + node->children[0]->identifier + " = " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::IDENTIFIER:
            result = node->identifier;
            break;
        case NodeType::BOOLEAN_LITERAL:
            result = (node->value == 1) ? "true" : "false";
            break;
        // New node types for logical and relational operators
        case NodeType::LESS_THAN:
            result = "(" + format(node->children[0], indentLevel, os) + " < " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::LESS_EQUAL:
            result = "(" + format(node->children[0], indentLevel, os) + " <= " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::GREATER_THAN:
            result = "(" + format(node->children[0], indentLevel, os) + " > " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::GREATER_EQUAL:
            result = "(" + format(node->children[0], indentLevel, os) + " >= " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::EQUAL:
            result = "(" + format(node->children[0], indentLevel, os) + " == " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::NOT_EQUAL:
            result = "(" + format(node->children[0], indentLevel, os) + " != " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::LOGICAL_AND:
            result = "(" + format(node->children[0], indentLevel, os) + " & " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::LOGICAL_OR:
            result = "(" + format(node->children[0], indentLevel, os) + " | " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::LOGICAL_XOR:
            result = "(" + format(node->children[0], indentLevel, os) + " ^ " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::MODULO:
            result = "(" + format(node->children[0], indentLevel, os) + " % " + format(node->children[1], indentLevel, os) + ")";
            break;
        case NodeType::PRINT:
            result = "print " + format(node->children[0], indentLevel, os);
            break;
        case NodeType::WHILE: {
            result = indent + "while " + format(node->children[0], 0, os) + " {\n";
            for (const auto& child : node->children[1]->children) { // Assuming the second child is the block containing statements
                result += format(child, indentLevel + 1, os);
                result += "\n";
            }
            result += indent + "}";
            break;
        }
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
    ostream& os = cout;
    std::string codeBlock;
    std::string inputLine;

    // Read all lines until EOF is received
    while (std::getline(std::cin, inputLine)) {
        codeBlock += inputLine + "\n"; // Add the line to the code block
    }

    // Process the entire code block as a single expression (which may contain multiple statements)
    try {
        Lexer lexer(codeBlock);
        auto tokens = lexer.tokenize();
        // Your error handling for lexer
        if (lexer.isSyntaxError(tokens)) {
            throw std::runtime_error("");
        }
        InfixParser parser(tokens);
        Node* root = parser.parse(os);
        os << format(root, 0, os) << std::endl;    // Output the formatted string
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown exception occurred." << std::endl;
    }

    return 0;
}