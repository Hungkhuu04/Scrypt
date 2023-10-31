
#include "lib/infixParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;

std::unordered_map<std::string, double> variables;
std::unordered_map<std::string, double> tempVariables;

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


/*Evaluates the expression stored in the AST through recursion and returns a value.
Throws errors when appropriate. */
std::string evaluate(Node* node, std::unordered_map<std::string, double>& variables) {
    if (!node) return "0";

    std::string result;
    switch (node->type) {
        case NodeType::NUMBER:
            return formatDecimal(node->value);
        case NodeType::ADD:
            return formatDecimal(stod(evaluate(node->children[0], variables)) + stod(evaluate(node->children[1], variables)));
        case NodeType::SUBTRACT:
            return formatDecimal(stod(evaluate(node->children[0], variables)) - stod(evaluate(node->children[1], variables)));
        case NodeType::MULTIPLY:
            return formatDecimal(stod(evaluate(node->children[0], variables)) * stod(evaluate(node->children[1], variables)));
        case NodeType::DIVIDE:
            return formatDecimal(stod(evaluate(node->children[0], variables)) / stod(evaluate(node->children[1], variables)));
        case NodeType::ASSIGN:
            variables[node->children[0]->identifier] = stod(evaluate(node->children[1], variables));
            return formatDecimal(variables[node->children[0]->identifier]);
        case NodeType::IDENTIFIER:
            return formatDecimal(variables[node->identifier]);
        case NodeType::LESS_THAN:
            return (stod(evaluate(node->children[0], variables)) < stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::LESS_EQUAL:
            return (stod(evaluate(node->children[0], variables)) <= stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::GREATER_THAN:
            return (stod(evaluate(node->children[0], variables)) > stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::GREATER_EQUAL:
            return (stod(evaluate(node->children[0], variables)) >= stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::EQUAL:
            return (stod(evaluate(node->children[0], variables)) == stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::NOT_EQUAL:
            return (stod(evaluate(node->children[0], variables)) != stod(evaluate(node->children[1], variables))) ? "true" : "false";
        case NodeType::LOGICAL_AND:
            return (evaluate(node->children[0], variables) == "true" && evaluate(node->children[1], variables) == "true") ? "true" : "false";
        case NodeType::LOGICAL_OR:
            return (evaluate(node->children[0], variables) == "true" || evaluate(node->children[1], variables) == "true") ? "true" : "false";
        case NodeType::LOGICAL_XOR:
            return ((evaluate(node->children[0], variables) == "true") ^ (evaluate(node->children[1], variables) == "true")) ? "true" : "false";
        default:
            return "Error: Unknown node type";
    }
    return result;
}


/*Takes in a node object and then returns the expression in infix form. Goes through the AST
recursively and builds the string representation off the stored expression .*/
std::string infixString(Node* node, std::ostream& os = std::cout) {
    if (!node) {
        return "";
    }
    
    std::string result;
    
    switch (node->type) {
        case NodeType::NUMBER:
            result = formatDecimal(node->value);  // assuming formatDecimal is defined elsewhere
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
        // New node types for logical and relational operators
        case NodeType::LESS_THAN:
            result = "(" + infixString(node->children[0], os) + " < " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LESS_EQUAL:
            result = "(" + infixString(node->children[0], os) + " <= " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::GREATER_THAN:
            result = "(" + infixString(node->children[0], os) + " > " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::GREATER_EQUAL:
            result = "(" + infixString(node->children[0], os) + " >= " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::EQUAL:
            result = "(" + infixString(node->children[0], os) + " == " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::NOT_EQUAL:
            result = "(" + infixString(node->children[0], os) + " != " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_AND:
            result = "(" + infixString(node->children[0], os) + " && " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_OR:
            result = "(" + infixString(node->children[0], os) + " || " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_XOR:
            result = "(" + infixString(node->children[0], os) + " ^^ " + infixString(node->children[1], os) + ")";
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
        std::unordered_map<std::string, double> tempVariables = variables; // Create a temporary copy
        try {
            Lexer lexer(inputLine);
            auto tokens = lexer.tokenize();
            // Your error handling for lexer
            if (lexer.isSyntaxError(tokens)) {
                throw std::runtime_error("");
            }
            InfixParser parser(tokens);
            Node* root = parser.parse(os);
            os << infixString(root, os) << endl;
            string result = evaluate(root, tempVariables); // Evaluate using the temporary copy
            variables = tempVariables; // Update the original variables if successful
            os << result << endl;

        } catch (const std::runtime_error& e) {
            os << e.what();
        } catch (...) {
            os << "An unknown exception occurred." << endl;
        }
    }
    return 0;
}