#ifndef PARSE_H
#define PARSE_H
#include "lex.h"
#include <vector>

using namespace std;

enum class NodeType {
    ADD, SUBTRACT, MULTIPLY, DIVIDE, NUMBER
};

struct Node {
    NodeType type;
    double value;
    vector<Node*> children;

    Node(NodeType t, double v = 0) : type(t), value(v) {}
};

class Parser {
private:
    vector<Token> tokens;
    int currentTokenIndex;
    Node* root;

    Token& currentToken();
    Node* expression();
    Node* number();

    void clearTree(Node* node);

public:
    Parser(const vector<Token>& tokens);
    ~Parser();

    Node* parse();
};

#endif
