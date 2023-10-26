#include "lib/infixParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;

std::unordered_map<std::string, double> variables;

/*Evaluates the expression stored in the AST through recursion and returns a value.
Throws errors when appropriate. */
double evaluate(Node* node, std::ostream& os = std::cerr) {
    if (!node) {
        os << "Error: Null node encountered while evaluating.\n";
        exit(1);
    }

    switch (node->type) {
        case NodeType::NUMBER:
            return node->value;
        case NodeType::ADD:
            return evaluate(node->children[0], os) + evaluate(node->children[1], os);

        case NodeType::SUBTRACT:
            return evaluate(node->children[0], os) - evaluate(node->children[1], os);

        case NodeType::MULTIPLY:
            return evaluate(node->children[0], os) * evaluate(node->children[1], os);

        case NodeType::DIVIDE:
            {
                double divisor = evaluate(node->children[1], os);
                if (divisor == 0) {
                    os << "Error: Division by zero.\n";
                    exit(1);
                }
                return evaluate(node->children[0], os) / divisor;
            }
        case NodeType::IDENTIFIER:
        {
            auto it = variables.find(node->identifier);
            if (it != variables.end()) {
                return it->second;
            } else {
                os << "Error: Undefined variable " << node->identifier << "\n";
                exit(1);
            }
        }
        case NodeType::ASSIGN:
        {
            if (node->children[0]->type != NodeType::IDENTIFIER) {
                os << "Error: Assignment must be to an identifier.\n";
                exit(1);
            }
            double value = evaluate(node->children[1], os);
            variables[node->children[0]->identifier] = value;
            return value;
        }
        default:
            os << "Error: Unknown node type encountered while evaluating.\n";
            exit(1);
    }
    return 0.0;  // Should not reach here.
}


//Takes in a value and converts the input into a usable string format.
// It ensures that there is the right amount of decimal points
string formatDecimal(double value) {
    if (value == static_cast<int>(value)) {
        return to_string(static_cast<int>(value));
    } else {
        ostringstream ss;
        ss << value;
        return ss.str();
    }
}

/*Takes in a node object and then returns the expression in infix form. Goes through the AST
recursively and builds the string representation off the stored expression .*/
string infixString(Node* node, std::ostream& os = std::cout) {
    if (!node) {
        return "";
    }
    
    string result;
    
    switch (node->type) {
        case NodeType::NUMBER:
            result = formatDecimal(node->value);
            break;
        case NodeType::ADD:
            result = "(" + infixString(node->children[0], os) + " + " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::SUBTRACT:
            result = "(" + infixString(node->children[0], os) + " - " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::MULTIPLY:
            result = "(" + infixString(node->children[0], os) + " * " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::DIVIDE:
            result = "(" + infixString(node->children[0], os) + " / " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::ASSIGN:
            result = "(" + node->children[0]->identifier + " = " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::IDENTIFIER:
            result = node->identifier;
            break;
        default:
            os << "Error: Unknown node type encountered while constructing infix string.\n";
            exit(1);
    }
    return result;
}

int main() {
    std::ostream& os = std::cout;
    string input;
    char ch;
    while (cin.get(ch)) {
        input += ch;
    }
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    InfixParser parser(tokens);
    Node* root = parser.parse(os);
    os << infixString(root, os) << std::endl;
    double result = evaluate(root, os);
    os << result << std::endl;
    return 0;
}
