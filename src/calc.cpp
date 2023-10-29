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
                    throw std::runtime_error("Runtime error: division by zero.");
                }
                return evaluate(node->children[0], os) / divisor;
            }
        case NodeType::IDENTIFIER:
        {
            auto it = variables.find(node->identifier);
            if (it != variables.end()) {
                return it->second;
            } else {
                throw std::runtime_error("Runtime error: unknown identifier " + node->identifier);
            }
        }
        case NodeType::ASSIGN:
        {
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
    ostream& os = cout;
    string inputLine;
    while (getline(cin, inputLine)) {
        if (inputLine.empty()) {
            continue;
        }
        
        try {  // <-- Begin try block
            Lexer lexer(inputLine);
            auto tokens = lexer.tokenize();
            InfixParser parser(tokens);
            Node* root = parser.parse(os);
            os << infixString(root, os) << endl;
            double result = evaluate(root, os);
            os << result << endl;
        } catch (const std::runtime_error& e) {  // <-- Catch exceptions
            os << e.what() << endl;
        } catch (...) { // Catch all other types of exceptions
            os << "An unknown exception occurred." << endl;
        }
    }
    return 0;
}