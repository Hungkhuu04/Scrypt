#include "lib/parse.h"
#include <iostream>
#include<string>
#include<sstream>
using namespace std;

/*Evaluates the expression stored in the AST through recursion and returns a value.
Throws errors when appropiate. */
double evaluate(Node* node, std::ostream& os = std::cerr) {
    switch (node->type) {
        case NodeType::NUMBER:
            return node->value;
        case NodeType::ADD: {
            double sum = 0;
            for (Node* child : node->children) {
                sum += evaluate(child, os);
            }
            return sum;
        }
        case NodeType::SUBTRACT: {
            double result = evaluate(node->children[0], os);
            for (size_t i = 1; i < node->children.size(); ++i) {
                result -= evaluate(node->children[i], os);
            }
            return result;
        }
        case NodeType::MULTIPLY: {
            double product = 1;
            for (Node* child : node->children) {
                product *= evaluate(child, os);
            }
            return product;
        }
        case NodeType::DIVIDE: {
            double result = evaluate(node->children[0], os);
            for (size_t i = 1; i < node->children.size(); ++i) {
                double divisor = evaluate(node->children[i], os);
                if (divisor == 0) {
                    os << "Runtime error: division by zero." << std::endl;
                    exit(3);
                }
                result /= divisor;
            }
            return result;
        }
        default:
            return 0;
    }
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


/*Takes in a node object and then returns the an expression in infix form. Goes through the AST
recursively and builds the string representation off the stored expression .*/
string infixString(Node* node, std::ostream& os = std::cout) {
    if (!node) return "";
    switch (node->type) {
        case NodeType::NUMBER:
            return formatDecimal(node->value);
        case NodeType::ADD:
        case NodeType::SUBTRACT:
        case NodeType::MULTIPLY:
        case NodeType::DIVIDE: {
            string result = "(";
            for (size_t i = 0; i < node->children.size(); ++i) {
                result += infixString(node->children[i], os);
                if (i != node->children.size() - 1) {
                    switch (node->type) {
                        case NodeType::ADD:
                            result += " + ";
                            break;
                        case NodeType::SUBTRACT:
                            result += " - ";
                            break;
                        case NodeType::MULTIPLY:
                            result += " * ";
                            break;
                        case NodeType::DIVIDE:
                            result += " / ";
                            break;
                        default:
                            break;
                    }
                }
            }
            result += ")";
            return result;
        }
        default:
            return "";
    }
}

/*
Reads the cin and creates the expression ready to send it to the parser.
The parser calls the tokensize function to create a token of each character. It adds the 
tokens to the AST and the prints out the answer using the evaluator to get the answer.
*/

int main() {
    std::ostream& os = std::cout;
    string input;
    char ch;
    while (cin.get(ch)) {
        input += ch;
    }
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    Node* root = parser.parse(os);
    os << infixString(root, os) << std::endl;
    double result = evaluate(root, os);
    os << result << std::endl;
    return 0;

}

