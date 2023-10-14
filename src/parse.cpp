#include "lib/parse.h"
#include <iostream>
#include<string>
using namespace std;

Parser::Parser(const vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

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
                cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << endl;
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
        Node* node = new Node(NodeType::NUMBER, stod(currentToken().value));
        currentTokenIndex++;
        return node;
    } else {
        cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << endl;
        exit(2);
    }
}

Node* Parser::parse() {
    Node* root = expression();
    if (currentToken().type != TokenType::UNKNOWN || currentToken().value != "END") {
        cerr << "Unexpected token at line " << currentToken().line << " column " << currentToken().column << ": " << currentToken().value << endl;
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
                    cerr << "Runtime error: division by zero." << endl;
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

string formatDouble(double value) {
    if (value == static_cast<int>(value)) {
        return to_string(static_cast<int>(value));
    } else {
        ostringstream ss;
        ss << value;
        return ss.str();
    }
}

string infixString(Node* node) {
    if (!node) return "";

    switch (node->type) {
        case NodeType::NUMBER:
            return formatDouble(node->value);
        case NodeType::ADD:
        case NodeType::SUBTRACT:
        case NodeType::MULTIPLY:
        case NodeType::DIVIDE: {
            string result = "(";
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
    string input;
    char ch;
    while (cin.get(ch)) {
        input += ch;
    }

    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    Node* root = parser.parse();

    cout << infixString(root) << endl;

    double result = evaluate(root);
    cout << result << endl;

    return 0;
} */
