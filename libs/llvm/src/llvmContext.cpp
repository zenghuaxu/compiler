//
// Created by LENOVO on 2024/11/14.
//

#include "../include/llvmContext.h"

void LLVMContext::print(std::ostream& out) {
    out << "declare i32 @getint()" << std::endl
        << "declare i32 @getchar()" << std::endl
        << "declare void @putint(i32)" << std::endl
        << "declare void @putch(i32)" << std::endl
        << "declare void @putstr(i8*)" << std::endl << std::endl;

    for (auto global: globals) {
        global->print_all(out);
    }
    for (auto function: functions) {
        function->print(out);
    }
    main_func->print(out);
}
