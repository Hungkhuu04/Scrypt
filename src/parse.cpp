#include "lib/parse.h"
#include <iostream>
#include<string>

double evaluate(Node* node) {
    switch (node->type) {
        case NodeType::NUMBER:
            return node->value;
        case NodeType::ADD: {
            double sum = 0;
            for (Node* child : node->children) {
                sum += evaluate(child);
            }
            return sum;
        }
        case NodeType::SUBTRACT: {
            double result = evaluate(node->children[0]);
            for (size_t i = 1; i < node->children.size(); ++i) {
                result -= evaluate(node->children[i]);
            }
            return result;
        }
        case NodeType::MULTIPLY: {
            double product = 1;
            for (Node* child : node->children) {
                product *= evaluate(child);
            }
            return product;
        }
        case NodeType::DIVIDE: {
            double result = evaluate(node->children[0]);
            for (size_t i = 1; i < node->children.size(); ++i) {
                double divisor = evaluate(node->children[i]);
                if (divisor == 0) {
                    std::cerr << "Runtime error: division by zero." << std::endl;
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

std::string formatDouble(double value) {
    if (value == static_cast<int>(value)) {
        return std::to_string(static_cast<int>(value));
    } else {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
}

std::string infixString(Node* node) {
    if (!node) return "";

    switch (node->type) {
        case NodeType::NUMBER:
            return formatDouble(node->value);
        case NodeType::ADD:
        case NodeType::SUBTRACT:
        case NodeType::MULTIPLY:
        case NodeType::DIVIDE: {
            std::string result = "(";
            for (size_t i = 0; i < node->children.size(); ++i) {
                result += infixString(node->children[i]);
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

int main() {
    std::string input;
    char ch;
    while (std::cin.get(ch)) {
        input += ch;
    }

    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    Node* root = parser.parse();

    std::cout << infixString(root) << std::endl;

    double result = evaluate(root);
    std::cout << result << std::endl;

    return 0;
} 


