#include "lib/mParser.h"
#include "lib/lex.h"
#include "lib/ASTNodes.h" 
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>


void evaluateBlock(const BlockNode* node);
void evaluateIf(const IfNode* node);
void evaluateWhile(const WhileNode* node);
void evaluatePrint(const PrintNode* node);
int evaluateExpression(const ASTNode* node); // Returns the evaluated value of an expression


int main() {
    std::ostream& os = std::cout;
    std::string line;
    std::string inputCode;
    while (std::getline(std::cin, line)) {
        inputCode += line + "\n";
    }

    try {
        Lexer lexer(inputCode);
        auto tokens = lexer.tokenize();
        if (lexer.isSyntaxError(tokens)) {
            throw std::runtime_error("");
        }
        Parser parser(tokens);
        auto ast = parser.parse();

        // Assuming BlockNode is the root of your AST
        if (ast->getType() == ASTNode::Type::BlockNode) {
            evaluateBlock(static_cast<const BlockNode*>(ast.get()));
        } else {
            throw std::runtime_error("");
        }

    } catch (const std::runtime_error& e) {
        os << e.what();
    }
    return 0;
}
