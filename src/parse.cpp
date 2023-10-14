#include "lib/parse.h"
#include <iostream>
#include<string>


Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

Token& Parser::currentToken() {
    return tokens[currentTokenIndex];
}

Node* Parser::expression() {
    if (currentToken().type == TokenType::LEFT_PAREN) {
        currentTokenIndex++;

        Node* node;
        switch (currentToken().type) {
            case TokenType::ADD:
                node = new Node(NodeType::ADD);
                break;
            case TokenType::SUBTRACT:
                node = new Node(NodeType::SUBTRACT);
                break;
            case TokenType::MULTIPLY:
                node = new Node(NodeType::MULTIPLY);
                break;
            case TokenType::DIVIDE:
                node = new Node(NodeType::DIVIDE);
                break;
            default:
                std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
                exit(2);
        }

        currentTokenIndex++;

        while (currentToken().type != TokenType::RIGHT_PAREN) {
            node->children.push_back(expression());
        }

        currentTokenIndex++;
        return node;
    } else {
        return number();
    }
}

Node* Parser::number() {
    if (currentToken().type == TokenType::NUMBER) {
        Node* node = new Node(NodeType::NUMBER, std::stod(currentToken().value));
        currentTokenIndex++;
        return node;
    } else {
        std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
    }
}

Node* Parser::parse() {
    Node* root = expression();
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        std::cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << std::endl;
        exit(2);
    }
    return root;
}

void Parser::clearTree(Node* node) {
    if (!node) return;

    for (Node* child : node->children) {
        clearTree(child);
    }

    delete node;
}

Parser::~Parser() {
    clearTree(parse());
}

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

/*int main() {
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
} */


