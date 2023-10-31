
#ifndef PARSE_H
#define PARSE_H
#include "lex.h"
#include <vector>
#include<iostream>

// Parser Header file

using namespace std;
enum class NodeType {
    ADD, SUBTRACT, MULTIPLY, DIVIDE, NUMBER, ASSIGN, IDENTIFIER
};
struct Node {
    NodeType type;
    double value;
    string identifier;
    vector<Node*> children;
    Node(NodeType t, double v = 0, const string& id = "") : type(t), value(v), identifier(id) {}
};

class Parser {
private:
    vector<Token> tokens;
    int currentTokenIndex;
    int currentLineNumber;
    Node* root;
    Token& currentToken();
    Node* expression(std::ostream& os = std::cerr);
    Node* number(std::ostream& os = std::cerr);
public:
    Parser(const vector<Token>& tokens, int lineCount);
    ~Parser();
    Node* parse(std::ostream& os = std::cerr);
    void clearTree(Node* node);
};

#endif