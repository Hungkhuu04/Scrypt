# Scrypt
Burk K T UCSB Github Repo for the Scrypt Project. Members Include: Kevin Hernandez, Andy Subramanian, and Hung Khuu

# Description
This project is a command-line calculator that can parse and calculate S-expressions. The project has been updated to allow inflix expressions and S-expressions together.

It's composed of four main parts:
- a lexer for to tokenize the input
- a parser for building the expression into an abstract syntax tree (AST)
- the parser also contains an evaluator for computing the result of the given expression.
- a calc file. The calc file is similar to the parser files but instead of taking an S-expression, it takes an inflix expression, computes and returns the answer.

# How to Use
There are two tests to be compiled.

To compile the Lexer the **program** uses:
- g++ -Wall -Wextra -Werror -o lexer_test parse.cpp lib/parser.cpp lib/lexer.cpp

To compile the Parser the **program** uses:
- g++ -Wall -Wextra -Werror -o parser_tets lex.cpp lib/parser.cpp lib/lexer.cpp

To complile the Calc fiel the **program** uses:
- g++ -Wall -Wextra -Werror -o calc_test calc.cpp lib/parser.cpp lib/lexer.cpp

Once the project is complied, you can use the **program**  to parse and evaluate mathematical expressions. The calculator takes an input from the standard input and outputs the result as an ostream.