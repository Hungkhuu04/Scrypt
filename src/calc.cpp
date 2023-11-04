
#include "lib/infixParser.h"
#include "lib/lex.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cmath>

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

bool isBooleanString(const std::string& str) {
    return str == "true" || str == "false";
}

bool isNumber(const std::string& s) {
    char* end;
    std::strtol(s.c_str(), &end, 10);
    return end == s.c_str() + s.size();
}


/*Evaluates the expression stored in the AST through recursion and returns a value.
Throws errors when appropriate. */
std::string evaluate(Node* node, std::unordered_map<std::string, double>& variables) {
    if (!node) return "0";
    std::string result;
    std::string left, right;
    try { 
        switch (node->type) {
            case NodeType::NUMBER:
                return formatDecimal(node->value);
            case NodeType::ADD:
                return formatDecimal(std::stod(evaluate(node->children[0], variables)) + std::stod(evaluate(node->children[1], variables)));
            case NodeType::SUBTRACT:
                return formatDecimal(std::stod(evaluate(node->children[0], variables)) - std::stod(evaluate(node->children[1], variables)));
            case NodeType::MULTIPLY:
                return formatDecimal(std::stod(evaluate(node->children[0], variables)) * std::stod(evaluate(node->children[1], variables)));
            case NodeType::DIVIDE:
                {
                    double divisor = std::stod(evaluate(node->children[1], variables));
                    if (divisor == 0) {
                        throw std::runtime_error("Runtime error: division by zero.\n");
                    }
                    return formatDecimal(std::stod(evaluate(node->children[0], variables)) / divisor);
                }
            case NodeType::IDENTIFIER:
                {
                    if (node->identifier == "true" || node->identifier == "t") { // Add "t" here
                        return "true";
                    } else if (node->identifier == "false" || node->identifier == "f") { // Add "f" here
                        return "false";
                    }
                    auto it = variables.find(node->identifier);
                    if (it != variables.end()) {
                        return formatDecimal(it->second);
                    } else {
                        throw std::runtime_error("Runtime error: unknown identifier " + node->identifier + '\n');
                    }
                }
            case NodeType::ASSIGN:
                {
                    string value = evaluate(node->children[1], variables);
                    if (value == "true") {
                        variables[node->children[0]->identifier] = 1;
                        return "true";
                    } else if (value == "false") {
                        variables[node->children[0]->identifier] = 0;
                        return "false";
                    } else {
                        double numericValue = std::stod(value);
                        variables[node->children[0]->identifier] = numericValue;
                        return formatDecimal(numericValue);
                    }
                }
            case NodeType::BOOLEAN_LITERAL:
                return (node->value == 1) ? "true" : "false";
            case NodeType::LESS_THAN:
                return (std::stod(evaluate(node->children[0], variables)) < std::stod(evaluate(node->children[1], variables))) ? "true" : "false";
            case NodeType::LESS_EQUAL:
                return (std::stod(evaluate(node->children[0], variables)) <= std::stod(evaluate(node->children[1], variables))) ? "true" : "false";
            case NodeType::GREATER_THAN:
                return (std::stod(evaluate(node->children[0], variables)) > std::stod(evaluate(node->children[1], variables))) ? "true" : "false";
            case NodeType::GREATER_EQUAL:
                return (std::stod(evaluate(node->children[0], variables)) >= std::stod(evaluate(node->children[1], variables))) ? "true" : "false";
            case NodeType::EQUAL:
                left = evaluate(node->children[0], variables);
                right = evaluate(node->children[1], variables);
                // If either side is a boolean string ("true" or "false")
                if (left == "true" || left == "false") {
                    return (left == (right == "1" ? "true" : (right == "0" ? "false" : right))) ? "true" : "false";
                } else if (right == "true" || right == "false") {
                    return (right == (left == "1" ? "true" : (left == "0" ? "false" : left))) ? "true" : "false";
                } else { // Both are numbers
                    return (std::stod(left) == std::stod(right)) ? "true" : "false";
                }

            case NodeType::NOT_EQUAL:
                left = evaluate(node->children[0], variables);
                right = evaluate(node->children[1], variables);
                // If either side is a boolean string ("true" or "false")
                if (left == "true" || left == "false") {
                    return (left != (right == "1" ? "true" : (right == "0" ? "false" : right))) ? "true" : "false";
                } else if (right == "true" || right == "false") {
                    return (right != (left == "1" ? "true" : (left == "0" ? "false" : left))) ? "true" : "false";
                } else { // Both are numbers
                    return (std::stod(left) != std::stod(right)) ? "true" : "false";
                }
            case NodeType::LOGICAL_AND:
                return (evaluate(node->children[0], variables) == "true" && evaluate(node->children[1], variables) == "true") ? "true" : "false";
            case NodeType::LOGICAL_XOR:
                left = evaluate(node->children[0], variables);
                right = evaluate(node->children[1], variables);
                
                // Check if one is boolean and the other is a number
                if ((isBooleanString(left) && !isBooleanString(right)) || (!isBooleanString(left) && isBooleanString(right))) {
                    throw std::runtime_error("Runtime error: invalid operand type.\n");
                }
                // Check if operands are either true, false, or valid numbers.
                if (!((left == "true" || left == "false" || isNumber(left)) &&
                    (right == "true" || right == "false" || isNumber(right)))) {
                    throw std::runtime_error("Runtime error: invalid operand type.\n");
                }
                
                if ((left == "true" && right == "false") || (left == "false" && right == "true")) {
                    return "true";
                }
                return "false";

            case NodeType::LOGICAL_OR:
                left = evaluate(node->children[0], variables);
                right = evaluate(node->children[1], variables);

                if ((isBooleanString(left) && !isBooleanString(right)) || (!isBooleanString(left) && isBooleanString(right))) {
                    throw std::runtime_error("Runtime error: invalid operand type.\n");
                }
                // Check if operands are either true, false, or valid numbers.
                if (!((left == "true" || left == "false" || isNumber(left)) &&
                    (right == "true" || right == "false" || isNumber(right)))) {
                    throw std::runtime_error("Runtime error: invalid operand type.\n");
                }

                return (left == "true" || right == "true") ? "true" : "false";
            case NodeType::MODULO:
            {
                double divisor = std::stod(evaluate(node->children[1], variables));
                if (divisor == 0) {
                    throw std::runtime_error("Runtime error: modulo by zero.\n");
                }
                return formatDecimal(std::fmod(std::stod(evaluate(node->children[0], variables)), divisor));
            }
            default:
                throw std::runtime_error("Unknown node type");
                }
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Runtime error: invalid operand type.\n");
    }
    return "0";
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
        case NodeType::BOOLEAN_LITERAL:
            result = (node->value == 1) ? "true" : "false";
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
            result = "(" + infixString(node->children[0], os) + " & " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_OR:
            result = "(" + infixString(node->children[0], os) + " | " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::LOGICAL_XOR:
            result = "(" + infixString(node->children[0], os) + " ^ " + infixString(node->children[1], os) + ")";
            break;
        case NodeType::MODULO:
            return "(" + infixString(node->children[0]) + " % " + infixString(node->children[1]) + ")";
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
            vector<Node*> roots = parser.parse(os); // Change to vector<Node*>
            for (Node* root : roots) { // Iterate through the vector of Node*
                if (root) {
                    os << infixString(root, os) << endl;
                    string result = evaluate(root, tempVariables); // Evaluate using the temporary copy
                    // Do something with the result
                    os << result << endl;
                }
                delete root; // Assuming the Node* were dynamically allocated, they need to be deleted to avoid memory leaks
            }
            variables = tempVariables; // Update the main variable map

        } catch (const std::runtime_error& e) {
            os << e.what();
        } catch (...) {
            os << "An unknown exception occurred." << endl;
        }
    }
    return 0;
}