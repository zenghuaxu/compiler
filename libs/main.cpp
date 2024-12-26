//
// Created by LENOVO on 2024/9/20.
//
#include <algorithm>
#include <iostream>
#include <fstream>

#include "include/configure.h"
#include "include/io.h"

#include "frontend/include/lexer.h"
#include "frontend/include/parser.h"

#include "frontend/include/visitor.h"

#include "llvm/include/ir/module.h"

#include "mips/include/mipsManager.h"

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
        OUTPUT_OPEN(output_lexer, lexer.txt)
        for (const Token& token : tokens) {
            output_lexer << token.toString() << std::endl;
        }
        output_lexer.close();
    #endif

    //syntactic analysis
    auto parser = Parser(tokens, errors);
    auto comp_unit_ptr = parser.parser();
    #ifdef  PARSER_
        OUTPUT_OPEN(output_parser, parser.txt)
        comp_unit_ptr->print(output_parser);
        output_parser.close();
    #endif

    //semantic analysis
    comp_unit_ptr->print(std::cout);
    auto visitor = Visitor(errors);
    auto module = visitor.visit(*comp_unit_ptr);

    #ifdef SEMANTIC_
        OUTPUT_OPEN(output_symbol, symbol.txt)
        visitor.print_symbol(output_symbol);
        output_symbol.close();
    #endif

    //error output
    std::sort(errors.begin(), errors.end());
    OUTPUT_OPEN(err, error.txt)
    for (Error error : errors) {
        err << error.toString() << std::endl;
    }
    err.close();

    if (!errors.empty()) {
        return 0;
    }
    module->emit_bb();
    module->mem2reg();
    module->dce();
    //module->lvn();
    // for (auto i = 0; i < 10; i++) {
    //     module->lvn();
    // }
    module->delete_phi();
    #ifdef LLVM_IR
    OUTPUT_OPEN(output, llvm_ir.txt);
    module->print(output);
    output.close();
    #endif

    auto mips = new MipsManager(module);
    mips->translate();
    mips->reduce_mul();
    mips->reduce_jump();

    OUTPUT_OPEN(mips_out, mips.txt);
    mips->print(mips_out);
    mips_out.close();
    return 0;
}
