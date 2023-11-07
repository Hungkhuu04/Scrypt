#include "lib/lex.h"
#include "lib/mParser.h"
#include <iostream>
#include <string>
using namespace std;

void printAST(mNode* node, std::ostream& os, int depth = 0) {
    if (!node) return;

    // Lambda function to print indentation
    auto printIndent = [&os](int level) {
        for (int i = 0; i < level; ++i) {
            os << "    "; // 4 spaces for each level of depth
        }
    };

    switch (node->type) {
        case mNodeType::ASSIGNMENT_STATEMENT:
            // Handle assignment separately if needed, to include the identifier name
            break;

        case mNodeType::IF_STATEMENT:
            printIndent(depth);
            os << "if (";
            printAST(node->condition, os, depth);
            os << ") {\n";
            printAST(node->thenBranch, os, depth + 1);
            if (node->elseBranch) {
                printIndent(depth);
                os << "} else {\n";
                printAST(node->elseBranch, os, depth + 1);
            }
            printIndent(depth);
            os << "}\n";
            break;

        case mNodeType::WHILE_STATEMENT:
            printIndent(depth);
            os << "while (";
            printAST(node->condition, os, depth);
            os << ") {\n";
            printAST(node->body, os, depth + 1);
            printIndent(depth);
            os << "}\n";
            break;

        case mNodeType::PRINT_STATEMENT:
            printIndent(depth);
            os << "print ";
            printAST(node->children.front(), os, depth);
            os << '\n';
            break;

        case mNodeType::BLOCK:
            for (auto& statement : node->statements) {
                printAST(statement, os, depth);
            }
            break;

        default:
            // By default, assume it is an expression node
            os << '(';
            for (auto& child : node->children) {
                printAST(child, os, depth);
            }
            os << ')';
            break;
    }
}


int main() {
    std::ostream& out_stream = std::cout; // This is where the AST will be printed
    std::ostream& err_stream = std::cerr; // This is where errors will be reported

    try {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf(); // Read the entire input into a string buffer
        std::string input = buffer.str();

        std::vector<Token> tokens = Lexer(input).tokenize(); // Tokenize the input
        mParser parser(tokens); // Create a parser with the tokens
        mNode* root = parser.parse(err_stream); // Parse the tokens into an AST, passing err_stream for error messages

        printAST(root, out_stream); // Print the AST, passing out_stream for output

        parser.clearTree(root); // Clean up the AST to avoid memory leaks
    } catch (const std::exception& e) {
        // Catch all exceptions derived from std::exception
        err_stream << "Error: " << e.what() << '\n';
        return 1; // Return a non-zero value to indicate an error
    } catch (...) {
        // Catch all other types of exceptions
        err_stream << "An unknown error occurred.\n";
        return 1; // Return a non-zero value to indicate an error
    }

    return 0;
}