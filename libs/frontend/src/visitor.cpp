//
// Created by LENOVO on 2024/10/14.
//

#include "../include/visitor.h"
#include "../../include/configure.h"

#include <cmath>

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


void Visitor::visit_number(Number& node) {
    auto content = node.token->getContent();
    std::stoi(content); //RETURN TODO
}

void Visitor::visit_character(Character& node) {
    node.token->getContent();
    //TODO 查表
}

void Visitor::visit_l_or_exp(LOrExp &node) {
    if (node.lhs) {
        visit_l_or_exp(*node.lhs);
    }
    visit_l_and_exp(*node.rhs);
}

void Visitor::visit_l_and_exp(LAndExp &node) {
    if (node.lhs) {
        visit_l_and_exp(*node.lhs);
    }
    visit_eq_exp(*node.rhs);
}

void Visitor::visit_eq_exp(EqExp &node) {
    if (node.lhs) {
        visit_eq_exp(*node.lhs);
    }
    visit_rel_exp(*node.rhs);
}

void Visitor::visit_rel_exp(RelExp &node) {
    if (node.lhs) {
        visit_rel_exp(*node.lhs);
    }
    std::shared_ptr<SymType> type;
    visit_add_exp(*node.rhs, type);
}

void Visitor::visit_add_exp(AddExp &node, std::shared_ptr<SymType> &type) {
    std::shared_ptr<SymType> l_type , r_type;
    if (node.lhs) {
        visit_add_exp(*node.lhs, l_type);
    }
    visit_mul_exp(*node.rhs, r_type);
    #ifdef DEBUG_SEMANTIC
    if (l_type) {
        std::cout << "add: l_type: ";
        l_type->print(std::cout);
        std::cout << std::endl;
    }
    std::cout << "add: r_type: ";
    r_type->print(std::cout);
    std::cout << std::endl;
    #endif
    assert(!l_type || l_type->match(r_type));
    type = r_type->evaluate();
}

void Visitor::visit_mul_exp(MulExp &node, std::shared_ptr<SymType> &type) {
    std::shared_ptr<SymType> l_type , r_type;
    if (node.lhs) {
        visit_mul_exp(*node.lhs, l_type);
    }
    visit_unary_exp(*node.rhs, r_type);
    assert(!l_type || l_type->match(r_type));
    type = r_type->evaluate();
}

void Visitor::visit_unary_exp(UnaryExp &node, std::shared_ptr<SymType> &type) {
    if (std::holds_alternative<CallExp>(node)) {
        const auto call = std::get_if<CallExp>(&node);
        visit_call_exp(*call, type);
    }
    else if (std::holds_alternative<OpUnaryExp>(node)) {
        //TODO +-!
        const auto unop = std::get_if<OpUnaryExp>(&node);
        visit_unary_exp(*unop->unary_exp, type);
    }
    else {//primary
        const auto primary = std::get_if<PrimaryUnaryExp>(&node);
        visit_primary_exp(*primary->primary, type);
    }
}

void Visitor::visit_primary_exp(PrimaryExp &node, std::shared_ptr<SymType> &type) {
    if (std::holds_alternative<Exp>(node)) {
        const auto exp = std::get_if<Exp>(&node);
        visit_add_exp(*exp->add_exp, type);
    }
    else if (std::holds_alternative<LVal>(node)) {
        const auto l_val = std::get_if<LVal>(&node);
        visit_l_val(*l_val, type);
    }
    else if (std::holds_alternative<Number>(node)) {
        const auto num = std::get_if<Number>(&node);
        visit_number(*num);
        type = std::make_shared<BasicType>
        (false, BasicType::int_type);
    }
    else {
        const auto ch = std::get_if<Character>(&node);
        visit_character(*ch);
        type = std::make_shared<BasicType>
        (false, BasicType::int_type);
    }
}

void Visitor::visit_call_exp(CallExp &node, std::shared_ptr<SymType> &type) {
    const auto symbol = current_sym_tab->get_from_all_scopes(
        node.identifier->getContent());

    if (symbol) {
        if (typeid(*symbol->get_type()) == typeid(FunctionType)) {
            const auto func =
                std::dynamic_pointer_cast<FunctionType>(symbol->get_type());
            type = func->evaluate(); //Call, not *func

            //params match check
            const auto params = func->get_params();
            //no r params
            if (!node.func_r_params) {
                if (!params.empty()) {
                    errors.emplace_back('d', node.identifier->getLine());
                }
                return;
            }
            if (node.func_r_params->exps.size() != params.size()) {
                errors.emplace_back('d', node.identifier->getLine());
            }
            //visit and check type
            int i = 0;
            for (const auto &exp:node.func_r_params->exps) {
                if (i >= params.size()) {
                    break;
                }
                std::shared_ptr<SymType> sym_type_ptr;
                visit_add_exp(*exp->add_exp, sym_type_ptr);
                if (!params.at(i) || !sym_type_ptr->match(params.at(i))) {
                    errors.emplace_back('e', node.identifier->getLine());
                    break;
                }
                i++;
            }
            return;
        }

        throw std::logic_error("Object symbol used as function symbol:" +
            node.identifier->getContent());
    }
    errors.emplace_back('c', node.identifier->getLine()); // not define
    type = std::make_shared<BasicType>(false, BasicType::int_type);
    //TODO type
}

void Visitor::visit(CompUnit &node) {
    for (auto &decl: node.decls) {
        visit_decl(*decl);
    }

    for (auto &func: node.func_defs) {
        visit_func(*func);
    }

    visit_main_func(*node.main_func_def);
}

void Visitor::visit_decl(Decl &node) {
    if (std::holds_alternative<ConstDecl>(node)) {
        ConstDecl const *c = std::get_if<ConstDecl>(&node);
        auto basic_type =
                std::make_shared<BasicType>(true, c->b_type->getType());
        for (auto &def: c->const_defs) {
            visit_const_def(*def, basic_type);
        }
    }
    else {
        VarDecl const *v = std::get_if<VarDecl>(&node);
        auto basic_type =
            std::make_shared<BasicType>(false, v->b_type->getType());
        for (auto &def: v->var_defs) {
            visit_var_def(*def, basic_type);
        }
    }
}

void Visitor::visit_const_def(ConstDef &node, const std::shared_ptr<BasicType>& basic_type) {
    std::shared_ptr<SymType> type;
    if (node.const_expr) {
        type = std::make_shared<ArrayType>(0, basic_type);//TODO SIZE
        std::shared_ptr<SymType> index_type;
        visit_add_exp(*node.const_expr->add_exp, index_type);//TODO store value
    }
    else {
        type = basic_type;
    }

    const auto symbol = std::make_shared<Symbol>
    (type, node.identifier->getContent(),
        node.identifier->getLine(), 0);//ID?

    if (current_sym_tab->add_symbol(symbol)) {}
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    if (!node.const_init_val) {
        //TODO INIT
        return;
    }
    for (auto &exp : node.const_init_val->const_exps) {
        std::shared_ptr<SymType> init_type;
        visit_add_exp(*exp->add_exp, init_type);
    }
    //TODO init

}

void Visitor::visit_var_def(VarDef &node, const std::shared_ptr<BasicType>& basic_type) {
    std::shared_ptr<SymType> type;
    if (node.const_exp) {
        std::shared_ptr<SymType> index_type;
        visit_add_exp(*node.const_exp->add_exp, index_type);
        type = std::make_shared<ArrayType>(0, basic_type);//TODO
    }
    else {
        type = basic_type;
    }

    auto symbol = std::make_shared<Symbol>
    (type, node.identifier->getContent(), node.identifier->getLine(), 0);//ID? Line

    if (current_sym_tab->add_symbol(symbol)) {}
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    if (!node.init_val) {
        //TODO INIT
        return;
    }
    for (auto &exp : node.init_val->exps) {
        std::shared_ptr<SymType> init_type;
        visit_add_exp(*exp->add_exp, init_type);
    }
    //TODO init

}

void Visitor::visit_stmt(Stmt &node, int if_return, bool if_for) {
    std::visit(overloaded{
            [this](LValWrapStmt &node) { visit_l_val_wrap_stmt(node);},
            [this, if_return, if_for](IfStmt &node) {
                visit_if_stmt(node, if_return, if_for);
            },
            [this, if_return, if_for](ForStmt &node) {
                visit_for_stmt(node, if_return, if_for);
            },
            [this, if_for](BreakStmt &node) {
                visit_break_stmt(node, if_for);
            },
            [this, if_for](ContinueStmt &node) {
                visit_continue_stmt(node, if_for);
            },
            [this, if_return](ReturnStmt &node) {
                visit_return_stmt(node, if_return);
            },
            [this](PrintfStmt &node) { visit_printf_stmt(node);},
            [this, if_return, if_for](Block &node) {
                push_table();
                visit_block(node, if_return, if_for);
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
    visit_l_val_with_no_evaluate(*node.l_val, l_type);
    visit_l_val_exp(*node.l_val_exp);

    assert(l_type);
    if (l_type->isConst()) {
        errors.emplace_back('h', node.l_val->identifier->getLine());
        return;
    }
    //TODO assign
}

void Visitor::visit_exp_stmt(ExpStmt &node) {
    std::shared_ptr<SymType> l_type;
    if (node.exp) {
        visit_add_exp(*node.exp->add_exp, l_type);
    }
}

void Visitor::visit_if_stmt(IfStmt &node, int if_return, bool if_for) {
    if (node.cond) {
        visit_l_or_exp(*node.cond->l_or_exp);
    }
    visit_stmt(*node.if_stmt, if_return, if_for);
    if (node.else_stmt) {
        visit_stmt(*node.else_stmt, if_return, if_for);
    }
}

void Visitor::visit_for_stmt(ForStmt &node, int if_return, bool if_for) {
    if (node.init) {
        visit_for__stmt(*node.init);
    }
    if (node.cond) {
        visit_l_or_exp(*node.cond->l_or_exp);
    }
    if (node.loop_stmt) {
        visit_for__stmt(*node.loop_stmt);
    }
    assert(node.body_stmt);
    visit_stmt(*node.body_stmt, if_return, true);
}

void Visitor::visit_for__stmt(ForStmt_ &node) {
    std::shared_ptr<SymType> l_type, exp_type;
    visit_l_val_with_no_evaluate(*node.l_val, l_type);
    visit_add_exp(*node.exp->add_exp, exp_type);

    if (l_type->isConst()) {
        errors.emplace_back('h', node.l_val->identifier->getLine());
    }
    //TODO assign
}

void Visitor::visit_break_stmt(BreakStmt &node, bool if_for) {
    if (!if_for) {
        errors.emplace_back('m', node.line);
    }
}

void Visitor::visit_continue_stmt(ContinueStmt &node, bool if_for) {
    if (!if_for) {
        errors.emplace_back('m', node.line);
    }
}

void Visitor::visit_return_stmt(ReturnStmt &node, bool if_return) {
    if (!if_return && node.exp) {
        errors.emplace_back('f', node.line);
    }
    if (node.exp) {
        std::shared_ptr<SymType> exp_type;
        visit_add_exp(*node.exp->add_exp, exp_type);
    }
}

void Visitor::visit_printf_stmt(PrintfStmt &node) {
    auto string = node.string_const->getContent();
    auto num = visit_str(string);
    if (num != node.exps.size()) {
        errors.emplace_back('l', node.line);
    }

    for (auto &exp: node.exps) {
        std::shared_ptr<SymType> par_type;
        visit_add_exp(*exp->add_exp, par_type);
    }
}

int Visitor::visit_str(const std::string &str) {
    assert(str.at(0) == '\"');
    auto sum = 0;
    for (int i = 1; i < str.size() - 1; i++) {
        if (str.at(i) == '%' &&
            (str.at(i + 1) == 'c' || str.at(i + 1) == 'd')) {
            sum++;
        }
    }
    return sum;
}

void Visitor::visit_l_val_with_no_evaluate
    (LVal &node, std::shared_ptr<SymType> &l_type) {
    const auto symbol =
        current_sym_tab->get_from_all_scopes(node.identifier->getContent());

    if (symbol) {
        l_type = symbol->get_type();
        //if is array, return its element type
        if (typeid(*symbol->get_type()) == typeid(ArrayType)) {
            assert(node.exp);
            auto array_type
                = std::dynamic_pointer_cast<ArrayType>(symbol->get_type());
            l_type = array_type->element_type();//get element
        }
    }
    else {
        errors.emplace_back('c', node.identifier->getLine());
        l_type = std::make_shared<BasicType>(false, BasicType::int_type);
    }
}

void Visitor::visit_l_val(LVal &node, std::shared_ptr<SymType> &type) {
    const auto symbol =
        current_sym_tab->get_from_all_scopes(node.identifier->getContent());

    if (symbol) {
        type = symbol->get_type()->evaluate();
        //if is array, return its element type
        if (typeid(*symbol->get_type()) == typeid(ArrayType)) {
            if (node.exp) {
                std::shared_ptr<SymType> index_type;
                visit_add_exp(*node.exp->add_exp, index_type);
                auto array_type =
                    std::dynamic_pointer_cast<ArrayType>(symbol->get_type());
                type = array_type->element_type();
            }
        }
    }
    else {
        errors.emplace_back('c', node.identifier->getLine());
        type = std::make_shared<BasicType>(false, BasicType::int_type);
    }
}

void Visitor::visit_l_val_exp(LValExp &node) {
    if (std::holds_alternative<Exp>(node)) {
        auto exp = std::get_if<Exp>(&node);
        std::shared_ptr<SymType> type;
        visit_add_exp(*exp->add_exp, type);
        //TODO CHECK
    }
    //TODO GETINT GETCHAR
}

void Visitor::visit_func(FuncDef &node) {
    const auto return_type =
        std::make_shared<BasicType>(false, node.func_type->type->getType());

    const auto func_type =
        std::make_shared<FunctionType>(return_type);

    const auto func_ptr = std::make_shared<Symbol>
        (func_type, node.identifier->getContent(), node.line, 0);//TODO

    if (current_sym_tab->add_symbol(func_ptr)) {}
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    push_table();
    if (node.func_f_params) {
        visit_func_f_params(*node.func_f_params, func_ptr);
    }

    const auto return_func =
        func_type->evaluate()->isVoid() ? BLOCK_WITHOUT_RETURN : BLOCK_WITH_RETURN;
    visit_block(*node.block, return_func, false);
    current_sym_tab = current_sym_tab->pop_scope();
}

void Visitor::visit_func_f_params
    (FuncFParams &node, const std::shared_ptr<Symbol>& symbol) {

    for (const auto &it: node.func_f_params) {
        const auto basic_type =
            std::make_shared<BasicType>(false, it->b_type->getType());
        std::shared_ptr<SymType> type;
        if (it->array_type) {
            type = std::make_shared<ArrayType>(0, basic_type);//TODO
        }
        else {
            type = basic_type;
        }

        //insert params to function symbol
        symbol->insert_parameter(type);

        auto param_symbol = std::make_shared<Symbol>
        (type, it->identifier->getContent(), it->b_type->getLine(), 0);
        if (current_sym_tab->add_symbol(param_symbol)) {}
        else {
            errors.emplace_back('b', it->b_type->getLine());
        }
    }
}

void Visitor::visit_block(Block &node, int if_return, bool if_for) {
    for (auto &item: node.block_items) {
        if (std::holds_alternative<Decl>(*item)) {
            const auto decl = std::get_if<Decl>(&*item);
            visit_decl(*decl);
        }
        else {
            const auto stmt = std::get_if<Stmt>(&*item);
            visit_stmt(*stmt, if_return ? BLOCK_PLAIN : BLOCK_WITHOUT_RETURN, if_for);
        }
    }
    //check return
    if (if_return == BLOCK_WITH_RETURN) {
        if (node.block_items.empty()) {
            errors.emplace_back('g', node.line);
            return;
        }
        auto &last = node.block_items.back();
        auto stmt = std::get_if<Stmt>(&*last);
        if (!stmt || !std::get_if<ReturnStmt>(stmt)) {
            errors.emplace_back('g', node.line);
        }
    }
}

void Visitor::visit_main_func(MainFuncDef &node) {
    push_table();
    visit_block(*node.block, BLOCK_WITH_RETURN, false);
    current_sym_tab = current_sym_tab->pop_scope();
}

void Visitor::print_symbol(std::ostream &out) {
    for (const auto& table : sym_tables) {
        table->print_table(out);
    }
}
