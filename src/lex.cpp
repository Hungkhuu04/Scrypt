#include "lib/lex.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

// lexer main used to test
/* Reads the cin and creates the expression ready to send it to the lexer.
The Lexer calls the tokensize function to create a token of each character.
If there is a error then print that, else print the line using iomanip for formatting
*/

int main() {
    string input;
    char ch;
    while (cin.get(ch)) {
        input += ch;
    }
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    for (const auto& token : tokens) {
        if (token.type == TokenType::UNKNOWN && token.value != "END") {
            cout << "Syntax error on line " << token.line << " column " << token.column << "." << endl;
            return 1;
        }
    }
    for (const auto& token : tokens) {
        cout << right << setw(4) << token.line << setw(5) << token.column << setw(2) << "  " << token.value << endl;
    }
    return 0;
}