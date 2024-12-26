//
// Created by LENOVO on 2024/11/24.
//
#include "../include/visitor.h"
#include "../../include/configure.h"

#include "../../llvm/include/ir/module.h"
#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/instructions.h"
#include "../../llvm/include/ir/basicBlock.h"
#include "../../llvm/include/ir/function.h"

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void Visitor::visit_decl(Decl &node, bool isGlobal) {
    if (std::holds_alternative<ConstDecl>(node)) {
        ConstDecl const *c = std::get_if<ConstDecl>(&node);
        auto basic_type =
                std::make_shared<BasicType>(true, c->b_type->getType());
        for (auto &def: c->const_defs) {
            visit_const_def(*def, basic_type, isGlobal);
        }
    }
    else {
        VarDecl const *v = std::get_if<VarDecl>(&node);
        auto basic_type =
            std::make_shared<BasicType>(false, v->b_type->getType());
        for (auto &def: v->var_defs) {
            visit_var_def(*def, basic_type, isGlobal);
        }
    }
}

ValuePtr Visitor::allocation(const std::shared_ptr<SymType>& type, bool isGlobal, bool isConst, std::string ident) {
    ValuePtr value = nullptr;
    if (isConst && typeid(*type) != typeid(ArrayType)) {
        return value;/*do nothing*/
    }
    else if (isGlobal) {
        value = new GlobalValue(type->toValueType(current_module->getContext()), ident);
    }
    else {
        value = new AllocaInstruction(type->toValueType(current_module->getContext()), current_basic_block);
        if (typeid(*type) == typeid(ArrayType) &&
                std::dynamic_pointer_cast<ArrayType>(type)->get_size() == 0/*WARNING: TO INDICATE IS A POINTER*/) {
            current_sym_tab->get_from_all_scopes(ident)->insert_addr_addr(value);
            return value;
        }
    }
    current_sym_tab->get_from_all_scopes(ident)->insert_addr(value);
    //NOTICE: GLOBAL VALUE OR POINTER
    return value;
}

void Visitor::visit_stmt(Stmt &node, int if_return, bool if_for, BasicBlockPtr break_return, BasicBlockPtr continue_return) {
    std::visit(overloaded{
            [this](LValWrapStmt &node) { visit_l_val_wrap_stmt(node);},
            [this, if_return, if_for, break_return, continue_return](IfStmt &node) {
                auto out = new_basic_block(current_basic_block->get_function());
                visit_if_stmt(node, if_return, if_for, break_return, continue_return, out);
            },
            [this, if_return, if_for](ForStmt &node) {
                auto out = new_basic_block(current_basic_block->get_function());
                visit_for_stmt(node, if_return, if_for, out);
            },
            [this, if_for, break_return](BreakStmt &node) {
                visit_break_stmt(node, if_for, break_return);
            },
            [this, if_for, continue_return](ContinueStmt &node) {
                visit_continue_stmt(node, if_for, continue_return);
            },
            [this, if_return](ReturnStmt &node) {
                visit_return_stmt(node, if_return);
            },
            [this](PrintfStmt &node) { visit_printf_stmt(node);},
            [this, if_return, if_for, break_return, continue_return](Block &node) {
                push_table();
                visit_block(node, if_return, if_for, break_return, continue_return);
                current_sym_tab = current_sym_tab->pop_scope();
            },
    }, node);
}

void Visitor::visit_l_val_wrap_stmt(LValWrapStmt& node) {
    assert(node.l_val_stmt);
    visit_l_val_stmt(*node.l_val_stmt);
}

void Visitor::visit_l_val_stmt(LValStmt &node) {
    std::visit(overloaded{
        [this](AssignStmt &node) {visit_assign_stmt(node);},
        [this](ExpStmt &node) {visit_exp_stmt(node);}
        },
        node);
}

void Visitor::visit_assign_stmt(AssignStmt &node) {
    std::shared_ptr<SymType> l_type;
    auto designate = visit_l_val_with_no_evaluate(*node.l_val, l_type);
    auto value = visit_l_val_exp(*node.l_val_exp);

    assert(l_type);
    if (l_type->isConst()) {
        errors.emplace_back('h', node.l_val->identifier->getLine());
        return;
    }

    if (designate) {
        value = type_conversion(value->get_value_return_type(), dynamic_cast<PointerTypePtr>(
            designate->get_value_return_type())->get_referenced_type(), value);
        // if (designate->get_value_return_type() == current_module->getContext()->getCharType()) {
        //     value = new TruncInstruction(value, current_basic_block);
        // }
        new StoreInstruction(value, designate, current_basic_block);
        //THIS IS HEAD NODE TODO
    }
}

void Visitor::visit_exp_stmt(ExpStmt &node) {
    std::shared_ptr<SymType> l_type;
    if (node.exp) {
        visit_add_exp(*node.exp->add_exp, l_type);
        //THIS IS HEAD NODE TODO
    }
}

void Visitor::visit_if_stmt(IfStmt &node, int if_return, bool if_for,
    BasicBlockPtr break_return, BasicBlockPtr continue_return,
    BasicBlockPtr out) {
    assert(node.cond);
    auto if_body = new_basic_block(current_basic_block->get_function());
    if (node.else_stmt) {
        auto else_body = new_basic_block(current_basic_block->get_function());
        visit_l_or_exp(*node.cond->l_or_exp, if_body, else_body);
        current_basic_block = if_body;
        visit_stmt(*node.if_stmt, if_return, if_for, break_return, continue_return);
        if (current_basic_block->enable_pad()) {
            new JumpInstruction(out, current_basic_block); //NOTICE: INSERT INTO IF_BODY
        }
        current_basic_block = else_body;
        visit_stmt(*node.else_stmt, if_return, if_for, break_return, continue_return);
        if (current_basic_block->enable_pad()) {
            new JumpInstruction(out, current_basic_block);
        }
    }
    else {
        visit_l_or_exp(*node.cond->l_or_exp, if_body, out);
        current_basic_block = if_body;
        visit_stmt(*node.if_stmt, if_return, if_for, break_return, continue_return);
        if (current_basic_block->enable_pad()) {
            new JumpInstruction(out, current_basic_block); //NOTICE: INSERT INTO IF_BODY
        }
    }
    current_basic_block = out;
}

void Visitor::visit_for_stmt(ForStmt &node, int if_return, bool if_for, BasicBlockPtr out) {
    if (node.init) {
        visit_for__stmt(*node.init);
    }
    auto entrance_block = current_basic_block;
    auto body_block = new_basic_block(current_basic_block->get_function());
    BasicBlockPtr cond_block = nullptr;
    current_basic_block = body_block;
    if (node.cond) {
        cond_block = new_basic_block(current_basic_block->get_function()); //COND BLOCK;
        if (entrance_block->enable_pad()) {
            new JumpInstruction(cond_block, entrance_block);
        }
        current_basic_block = cond_block;
        visit_l_or_exp(*node.cond->l_or_exp, body_block, out);
    }
    else if (entrance_block->enable_pad()) {
        new JumpInstruction(body_block, entrance_block);
    }

    auto return_block = current_basic_block;
    auto loop_block = new_basic_block(current_basic_block->get_function());
    //NOW IN LOOP STMT RETURN BLOCK
    if (node.loop_stmt) {
        current_basic_block = loop_block;//LOOP BLOCK
        visit_for__stmt(*node.loop_stmt);
    }
    new JumpInstruction(return_block, loop_block);

    assert(node.body_stmt);
    return_block = current_basic_block;
    current_basic_block = body_block;
    auto break_return = out;
    auto continue_return = loop_block;

    visit_stmt(*node.body_stmt, if_return, true, break_return, continue_return);
    if (current_basic_block->enable_pad()) {
        new JumpInstruction(return_block, current_basic_block);
    }
    current_basic_block = out;
}

void Visitor::visit_for__stmt(ForStmt_ &node) {
    std::shared_ptr<SymType> l_type, exp_type;
    auto designate = visit_l_val_with_no_evaluate(*node.l_val, l_type);
    auto value = visit_add_exp(*node.exp->add_exp, exp_type);

    if (l_type->isConst()) {
        errors.emplace_back('h', node.l_val->identifier->getLine());
        return;
    }

    if (designate) {
        value = type_conversion(value->get_value_return_type(), dynamic_cast<PointerTypePtr>(
            designate->get_value_return_type())->get_referenced_type(), value);
        // if (designate->get_value_return_type() == current_module->getContext()->getCharType()) {
        //     value = new TruncInstruction(value, current_basic_block);
        // }
        new StoreInstruction(value, designate, current_basic_block);
        //THIS IS HEAD NODE TODO
    }
}

void Visitor::visit_break_stmt(BreakStmt &node, bool if_for, BasicBlockPtr return_block) {
    if (!if_for) {
        errors.emplace_back('m', node.line);
        return;
    }
    assert(return_block);
    new JumpInstruction(return_block, current_basic_block);
    current_basic_block = new_basic_block(current_basic_block->get_function());
}

void Visitor::visit_continue_stmt(ContinueStmt &node, bool if_for, BasicBlockPtr return_block) {
    if (!if_for) {
        errors.emplace_back('m', node.line);
        return;
    }
    assert(return_block);
    new JumpInstruction(return_block, current_basic_block);
    current_basic_block = new_basic_block(current_basic_block->get_function());
}

void Visitor::visit_return_stmt(ReturnStmt &node, bool if_return) {
    if (!if_return && node.exp) {
        errors.emplace_back('f', node.line);
        return;
    }
    ValuePtr ret_value = new Constant(0, current_module->getContext()->getIntType());
    if (node.exp) {
        std::shared_ptr<SymType> exp_type;
        ret_value = visit_add_exp(*node.exp->add_exp, exp_type);
    }
    auto func = current_basic_block->get_function();
    ret_value =
        type_conversion(ret_value->get_value_return_type(), func->get_value_return_type(), ret_value);
    new ReturnInstruction(current_module->getContext()->getVoidType(), func, ret_value, current_basic_block);
    //END OF BB
}

void Visitor::visit_printf_stmt(PrintfStmt &node) {
    auto string = node.string_const->getContent();
    auto is_int_vec = visit_str(string);
    if (is_int_vec.size() != node.exps.size()) {
        errors.emplace_back('l', node.line);
        for (auto &exp: node.exps) {
            std::shared_ptr<SymType> par_type;
            visit_add_exp(*exp->add_exp, par_type);
        }
        return;
        //TODO 冗余？
    }

    int begin = 1;
    putstr(begin, string);
    for (auto &exp: node.exps) {
        std::shared_ptr<SymType> par_type;
        auto value = visit_add_exp(*exp->add_exp, par_type);
        switch (string.at(begin - 1)) {
            case 'c': {
                auto out = type_conversion(value->get_value_return_type(),
                   current_module->getContext()->getCharType(), value);
                out = new ZextInstruction(out, current_basic_block);
                new OutputInstruction(out, current_basic_block, true);
                break;
            }
            default: {
                auto out = type_conversion(value->get_value_return_type(),
                   current_module->getContext()->getIntType(), value);
                new OutputInstruction(out, current_basic_block);
            }
        }
        putstr(begin, string);
    }
}

std::vector<bool> Visitor::visit_str(const std::string &str) {
    std::vector<bool> result;
    assert(str.at(0) == '\"');
    for (int i = 1; i < str.size() - 1; i++) {
        if (str.at(i) == '%' &&
            (str.at(i + 1) == 'c' || str.at(i + 1) == 'd')) {
            result.push_back(str.at(i) == 'd');
        }
    }
    return result;
}

std::string Visitor::get_string_prefix(int &begin, std::string &str) {
    #ifdef IR_DEBUG
    std::cout << begin;
    #endif
    int a = begin;
    int i;
    std::string str_prefix;
    for (i = begin; i < str.size() - 1; i++) {
        if (str.at(i) == '%' &&
                (str.at(i + 1) == 'c' || str.at(i + 1) == 'd')) {
                begin = i + 2;
                break;
        }
        if (str.at(i) == '\\') {
            int value = char_to_value.at(str.substr(i, 2));
            str_prefix.push_back(value);
            i++;
        }
        else {
            str_prefix.push_back(str.at(i));
        }
    }
    return str_prefix;
}

void Visitor::putstr(int &begin, std::string &string) {
    auto string_prefix = get_string_prefix(begin, string);
    if (!string_prefix.empty()) {
        auto str_item_addr = new GlobalValue(current_module->getContext()->getArrayType(
            current_module->getContext()->getCharType(), string_prefix.size() + 1), get_string_name());
        str_item_addr->set_string(string_prefix);
        //ALLOCA CHAR ARRAY
        auto str_item = new GetElementPtrInstruction(str_item_addr, new Constant(0, current_module->getContext()->getIntType()),
            current_basic_block);
        new OutputInstruction(str_item, current_basic_block);
    }
}
