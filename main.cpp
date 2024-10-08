//
// Created by LENOVO on 2024/9/20.
//
#include <algorithm>
#include <iostream>
#include <fstream>

#include  "include/configure.h"
#include  "include/lexer.h"
#include "include/parser.h"

#define INPUT_OPEN(x)      std::ifstream x("testfile.txt");\
                           if (!x.is_open()) {\
                                std::cerr << "Error opening file" << std::endl;\
                                return -1;\
                           }

#define OUTPUT_OPEN(x, y)    std::ofstream x(#y);\
                             if (!x.is_open()) {\
                                std::cerr << "Error opening file" << std::endl;\
                                return -1;\
                             }

int main() {
    //open src file
    INPUT_OPEN(input)

    //error handling
    std::vector<Error> errors;

    //lexical analysis
    std::vector<Token> tokens;
    auto lexer = Lexer(tokens, input, errors);
    while (!input.eof()) {
        lexer.nextToken();
    }
    input.close();

    #ifdef LEXER_
        OUTPUT_OPEN(output, lexer.txt);
        for (const Token& token : tokens) {
            output << token.toString() << std::endl;
        }
        output.close();
    #endif

    //syntactic analysis
    #ifdef  PARSER_
        OUTPUT_OPEN(output, parser.txt);
        auto parser = Parser(tokens, errors);
        parser.parser()->print(output);
        output.close();
    #endif

    std::sort(errors.begin(), errors.end());
    OUTPUT_OPEN(err, error.txt)
    for (Error error : errors) {
        err << error.toString() << std::endl;
    }
    err.close();

    return 0;
}
