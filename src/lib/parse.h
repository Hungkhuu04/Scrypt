#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include <vector>
#include<string>


enum class NodeType {
    ADD, SUBTRACT, MULTIPLY, DIVIDE, NUMBER
};

struct Node {
    NodeType type;
    double value;
    std::vector<Node*> children;

    Node(NodeType t, double v = 0) : type(t), value(v) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;

    Token& currentToken();
    Node* expression();
    Node* number();

    void clearTree(Node* node);

public:
    Parser(const std::vector<Token>& tokens);
    ~Parser();

    Node* parse();
};

#endif
