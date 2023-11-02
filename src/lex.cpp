
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

    // Only calling isSyntaxError once to check for errors
    if (lexer.isSyntaxError(tokens)) {
        exit(1);
    }

    // Printing the tokens
    for (const auto& token : tokens) {
        cout << right << setw(4) << token.line << setw(5) << token.column << setw(2) << "  " << token.value << endl;
    }

    return 0;
}
