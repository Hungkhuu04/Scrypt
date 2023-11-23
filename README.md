 Scrypt

Burk K T UCSB Github Repo for the Scrypt Project. Members Include: Kevin Hernandez, Andy Subramanian, and Hung Khuu


# Description

This project is a command-line calculator that can parse and calculate S-expressions. The project has been updated to allow inflix expressions and S-expressions together. The projected has now been updated to take in blocks of statements and inflix expressions.


It's composed of four main parts:

- A lexer for to tokenize the input

- A parser for building the expression into an abstract syntax tree (AST)

- The parser also contains an evaluator for computing the result of the given expression.

- A calc file. The calc file is similar to the parser files but instead of taking an S-expression, it takes an inflix expression, computes and returns the answer.

- New parser which is a modified version of the old parser to allow multiple statements to be parsed.
  
- A format file for parsiing blocks of statements rather than individual expressions. Unlike calc it doesn't output the correct evaluation but rather the correct formating to ensure that there is proper indentation and parentheses especially for blocked statements such as IF,ELSE and WHILE. It is now updated to include functions definitions, function calls and array literals.

- A Scrpyt file. This is used to evaulate blocks of statements and expressions. It is now updated to include functions definitions, function calls and array literals.


# How to Use

There are two tests to be compiled.


To compile the **Lexer** the program uses:

- g++ -Wall -Wextra -Werror -o lexer_test parse.cpp lib/parser.cpp lib/lexer.cpp


To compile the **Parser** the program uses:

- g++ -Wall -Wextra -Werror -o parser_test parse.cpp lib/parser.cpp lib/lexer.cpp


To complile the **Calc** file the program uses:

- g++ -Wall -Wextra -Werror -o calc_test calc.cpp lib/infixParser.cpp lib/lexer.cpp lib/value.cpp


To complile the **Format** file the program uses:
- g++ -Wall -Wextra -Werror -o format_test format.cpp lib/mParser.cpp lib/lexer.cpp


To complile the **Scrypt** file the program uses:
- g++ -Wall -Wextra -Werror -o scrypt_test scrypt.cpp lib/mParser.cpp lib/lexer.cpp lib/value.cpp


Once the project is complied, you can use the programs**  to parse and evaluate mathematical expressions and blocks of statements. The program takes an input from the standard input and outputs the result as an ostream.