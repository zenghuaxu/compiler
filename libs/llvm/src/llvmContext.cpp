//
// Created by LENOVO on 2024/11/14.
//

#include "../include/llvmContext.h"
#include "../include/ir/function.h"

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

void LLVMContext::mem2reg() {
    for (auto func: functions) {
        func->create_dom();
        func->create_dom_tree();
        func->cal_DF();
        func->insert_phi();
        func->rename();
    }

    main_func->create_dom();
    main_func->create_dom_tree();
    main_func->cal_DF();
    main_func->insert_phi();
    main_func->rename();
}

void LLVMContext::emit_bb() {
    for (auto func: functions) {
        func->emit_blocks();
    }

    main_func->emit_blocks();
}

void LLVMContext::delete_phi() {
    for (auto func: functions) {
        func->delete_phi();
    }
    main_func->delete_phi();
}

void LLVMContext::dce() {
    for (auto func: functions) {
        func->mark_active_for_dce();
        func->dce();
    }
    main_func->mark_active_for_dce();
    main_func->dce();
}

void LLVMContext::lvn() {
    for (auto func: functions) {
        func->lvn();
    }
    main_func->lvn();
}