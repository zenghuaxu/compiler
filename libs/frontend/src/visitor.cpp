//
// Created by LENOVO on 2024/10/14.
//

#include "../include/visitor.h"
#include "../../include/configure.h"

#include <cmath>

#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/instructions.h"
#include "../../llvm/include/ir/function.h"
#include "../../llvm/include/ir/value.h"
#include "../../llvm/include/ir/globalValue.h"

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


ValuePtr Visitor::visit_number(Number &node) {
    auto content = node.token->getContent();
    int num = std::stoi(content);
    return new Constant(num, current_module->getContext()->getIntType());
}

ValuePtr Visitor::visit_character(Character &node) {
    node.token->getContent();
    auto ch = node.get_value();
    return new Constant(ch, current_module->getContext()->getIntType());
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

/**
 *
 * @param origin origin type
 * @param target target type
 * @param value object need to alter type
 * @return instruction
 * NOTICE:: CHAR->INT ZERO EXTENSION?
 */
ValuePtr Visitor::type_conversion(ValueReturnTypePtr origin, ValueReturnTypePtr target, ValuePtr value) {
    if (origin != target) {
        if (typeid(*value) == typeid(Constant)) {
            return new Constant(dynamic_cast<ConstantPtr>(value)->get_value(), target);
        }
        //TODO MORE COMPLICATED accommodation
        if (target->isInt()) {
            return new ZextInstruction(value, current_basic_block);
        }
        return new TruncInstruction(value, current_basic_block);
    }
    return value;
}

ValuePtr Visitor::visit_add_exp(AddExp &node, std::shared_ptr<SymType> &type) {
    std::shared_ptr<SymType> l_type , r_type;
    if (!node.lhs) {
        return visit_mul_exp(*node.rhs, type);
    }
    //HAS INSTRUCTION
    auto left = visit_add_exp(*node.lhs, l_type);
    auto right = visit_mul_exp(*node.rhs, r_type);

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
    type = l_type->isConst() ? r_type->evaluate() : l_type->evaluate();

    //CONSTANT INFERENCE
    auto constant_l = dynamic_cast<ConstantPtr>(left);
    auto constant_r = dynamic_cast<ConstantPtr>(right);
    if (constant_l && constant_r) {
        return Constant::merge(constant_l, constant_r, node.op->getType());
    }

    auto value_return_type = current_module->getContext()->getIntType();
    left = type_conversion(left->get_value_return_type(), value_return_type, left);
    right = type_conversion(right->get_value_return_type(), value_return_type, right);
    return new BinaryInstruction(value_return_type, node.op->getType(),
        left, right, current_basic_block);
}

ValuePtr Visitor::visit_mul_exp(MulExp &node, std::shared_ptr<SymType> &type) {
    std::shared_ptr<SymType> l_type , r_type;
    if (!node.lhs) {
        return visit_unary_exp(*node.rhs, type);
    }
    //has instr
    auto left = visit_mul_exp(*node.lhs, l_type);
    auto right = visit_unary_exp(*node.rhs, r_type);
    assert(!l_type || l_type->match(r_type));
    type = l_type->isConst() ? r_type->evaluate() : l_type->evaluate();

    //CONSTANT INFERENCE
    //CONSTANT INFERENCE
    auto constant_l = dynamic_cast<ConstantPtr>(left);
    auto constant_r = dynamic_cast<ConstantPtr>(right);
    if (constant_l && constant_r) {
        return Constant::merge(constant_l, constant_r, node.op->getType());
    }

    auto value_return_type = current_module->getContext()->getIntType();
    left = type_conversion(left->get_value_return_type(), value_return_type, left);
    right = type_conversion(right->get_value_return_type(), value_return_type, right);
    return new BinaryInstruction(value_return_type, node.op->getType(),
        left, right, current_basic_block);
}

ValuePtr Visitor::visit_unary_exp(UnaryExp &node, std::shared_ptr<SymType> &type) {
    if (std::holds_alternative<CallExp>(node)) {
        const auto call = std::get_if<CallExp>(&node);
        auto call_value = visit_call_exp(*call, type);
        // current_basic_block = new BasicBlock(current_module->getContext()->getVoidType(),
        //     current_basic_block->get_function());
        //TODO HOW TO GO BACK??
        //NEED TO CREATE A BB? NO NEED
        //DO NOT TYPE CONVERT HERE TODO
        // call_value = type_conversion(call_value->get_value_return_type(),
        //         current_module->getContext()->getIntType(), call_value);
        return call_value;
    }
    if (std::holds_alternative<OpUnaryExp>(node)) {
        const auto unop = std::get_if<OpUnaryExp>(&node);
        auto unary = visit_unary_exp(*unop->unary_exp, type);
        if (typeid(*unary) == typeid(Constant)) {
            return new Constant(-dynamic_cast<ConstantPtr>(unary)->get_value(), current_module->getContext()->getIntType());
        }
        if (!(*unop->unary_op->op_token == TokenType::PLUS)) {
            unary = type_conversion(unary->get_value_return_type(),
                current_module->getContext()->getIntType(), unary);
            return new UnaryOpInstruction(current_module->getContext()->getIntType(),
                                          unop->unary_op->op_token->getType(), unary, current_basic_block);
        }
        return unary;
    }
    //primary
    const auto primary = std::get_if<PrimaryUnaryExp>(&node);
    return visit_primary_exp(*primary->primary, type);
}

ValuePtr Visitor::visit_primary_exp(PrimaryExp &node, std::shared_ptr<SymType> &type) {
    if (std::holds_alternative<Exp>(node)) {
        const auto exp = std::get_if<Exp>(&node);
        return visit_add_exp(*exp->add_exp, type);
    }
    if (std::holds_alternative<LVal>(node)) {
        const auto l_val = std::get_if<LVal>(&node);
        return visit_l_val(*l_val, type);
    }
    if (std::holds_alternative<Number>(node)) {
        const auto num = std::get_if<Number>(&node);
        type = std::make_shared<BasicType>
                (true, BasicType::int_type);
        return visit_number(*num);
    }
    const auto ch = std::get_if<Character>(&node);
    type = std::make_shared<BasicType>
            (true, BasicType::int_type);
    return visit_character(*ch);
}

ValuePtr Visitor::visit_call_exp(CallExp &node, std::shared_ptr<SymType> &type) {
    const auto symbol = current_sym_tab->get_from_all_scopes(
        node.identifier->getContent());

    if (symbol) {
        if (typeid(*symbol->get_type()) == typeid(FunctionType)) {
            const auto func =
                std::dynamic_pointer_cast<FunctionType>(symbol->get_type());
            type = func->evaluate(); //Call, not *func

            auto return_type = func->get_return_type(current_module->getContext());

            //params match check
            const auto params = func->get_params();
            //no r params
            if (!node.func_r_params) {
                if (!params.empty()) {
                    errors.emplace_back('d', node.identifier->getLine());
                    return new Constant(0, current_module->getContext()->getIntType());
                }
                return new CallInstruction(return_type,
                    dynamic_cast<FunctionPtr>(symbol->get_addr()), current_basic_block);
            }
            if (node.func_r_params->exps.size() != params.size()) {
                errors.emplace_back('d', node.identifier->getLine());
            }
            //visit and check type
            int i = 0;
            std::vector<ValuePtr> values;
            for (const auto &exp:node.func_r_params->exps) {
                if (i >= params.size()) {
                    return new Constant(0, current_module->getContext()->getIntType());
                }
                std::shared_ptr<SymType> sym_type_ptr;
                auto value = visit_add_exp(*exp->add_exp, sym_type_ptr);
                if (!params.at(i) || !sym_type_ptr->match(params.at(i))) {
                    errors.emplace_back('e', node.identifier->getLine());
                    return new Constant(0, current_module->getContext()->getIntType());
                }
                value = type_conversion(value->get_value_return_type(),
                    dynamic_cast<FunctionPtr>(symbol->get_addr())->get_arg_return_type(i),
                    value);
                values.push_back(value);
                i++;
            }
            auto call = new CallInstruction(return_type,
                    dynamic_cast<FunctionPtr>(symbol->get_addr()), current_basic_block);
            for (auto value: values) {
                call->insert_parameter(value);//TODO CHECK ARRAY
            }
            return call;
        }

        throw std::logic_error("Object symbol used as function symbol:" +
            node.identifier->getContent());
    }
    errors.emplace_back('c', node.identifier->getLine()); // not define
    type = std::make_shared<BasicType>(false, BasicType::int_type);
    //TODO type
    return new Constant(0, current_module->getContext()->getIntType());;//TODO CHECK;
}

void Visitor::visit(CompUnit &node) {
    for (auto &decl: node.decls) {
        visit_decl(*decl, true);
    }

    for (auto &func: node.func_defs) {
        visit_func(*func);
    }

    visit_main_func(*node.main_func_def);
#ifdef IR_DEBUG
    current_module->print(std::cout);
#endif
}

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

ValuePtr Visitor::allocation(std::shared_ptr<SymType> type, bool isGlobal, bool isConst, std::string ident) {
    ValuePtr value = nullptr;
    if (isConst && typeid(*type) != typeid(ArrayType)) {/*do nothing*/}
    else if (isGlobal) {
        value = new GlobalValue(type->toValueType(current_module->getContext()), ident);
    }
    else {
        value = new AllocaInstruction(type->toValueType(current_module->getContext()), current_basic_block);
        //TODO STILL WRONG, WHEN ALLOC FOR A POINTER, TO DO THE FOLLOW, ELSE (ARRAY), THE UPPER.
        if (typeid(*type) == typeid(ArrayType) &&
                std::dynamic_pointer_cast<ArrayType>(type)->get_size() == 0) {
            current_sym_tab->get_from_all_scopes(ident)->insert_addr_addr(value);
            return value;
        }
    }
    current_sym_tab->get_from_all_scopes(ident)->insert_addr(value);
    //NOTICE: GLOBAL VALUE OR POINTER
    return value;
}

template <typename T>
void Visitor::init_global_object(ValuePtr value, std::shared_ptr<Symbol> symbol,
    T &const_exps, std::string string_const) {
    //TODO VALUE IS NULL
    auto global_value = dynamic_cast<GlobalValue*>(value);
    for (auto &exp : const_exps) {
        std::shared_ptr<SymType> init_type;
        auto init_val = visit_add_exp(*exp->add_exp, init_type);
        assert(init_type->isConst());
        auto constant = dynamic_cast<ConstantPtr>(init_val);
        if (global_value) {global_value->set_int(constant->get_value());}
        else {
            symbol->insert_value(constant);
        }
    }
    #ifdef IR_DEBUG
    std::cout << string_const << std::endl;
    #endif
    if (global_value) {
        std::string string_sub = std::string();
        for (int i = 1; i < static_cast<int>(string_const.size()) - 1; i++) {
            if (string_const.at(i) == '\\') {
                int value = char_to_value.at(string_const.substr(i, 2));
                string_sub.push_back(value);
                i++;
            }
            else {
                string_sub.push_back(string_const.at(i));
            }
        }
        global_value->set_string(string_sub);
    }
}

template <typename T1, typename T2>
void Visitor::init_local_object(ValuePtr value, std::shared_ptr<Symbol> symbol,
    T1 type, T2 &const_exps, std::string string_const) {
    std::shared_ptr<SymType> init_type;
    switch (type) {
        case ConstInitVal::constExp: {
            auto init = visit_add_exp(*const_exps.at(0)->add_exp, init_type);
            if (!value) {
                assert(typeid(*init) == typeid(Constant));
                auto constant = dynamic_cast<ConstantPtr>(init);
                symbol->insert_value(constant);
                //TODO BRACEDCONSTEXP 也有可能
                return;
            }
            init = type_conversion(init->get_value_return_type(),
                     dynamic_cast<PointerTypePtr>(value->get_value_return_type())->get_referenced_type(), init);
            new StoreInstruction(init, value, current_basic_block);
            break;
        }
        case ConstInitVal::bracedConstExp: {
//TODO CHECK
            auto type_ptr = std::dynamic_pointer_cast<ArrayType>(symbol->get_type());
            if(!type_ptr) {
                new StoreInstruction(new Constant(0,
                    dynamic_cast<PointerTypePtr>(value->get_value_return_type())->get_referenced_type()),
                    symbol->get_addr(), current_basic_block);
                return;
            }
            unsigned int size;
            size = type_ptr->get_size();
            int i = 0;
            for (auto &exp : const_exps) {
                auto init_val = visit_add_exp(*exp->add_exp, init_type);
                auto addr = new GetElementPtrInstruction(value, new Constant(i, current_module->getContext()->getIntType()), current_basic_block);
                init_val = type_conversion(init_val->get_value_return_type(),
                    dynamic_cast<PointerTypePtr>(addr->get_value_return_type())->get_referenced_type(), init_val);
                new StoreInstruction(init_val, addr, current_basic_block);
                i++;
            }
            for (;i < size; ++i) {
                auto addr = new GetElementPtrInstruction(value, new Constant(i, current_module->getContext()->getIntType()), current_basic_block);
                auto element_type = dynamic_cast<PointerTypePtr>(addr->get_value_return_type())->get_referenced_type();
                new StoreInstruction(new Constant(0, element_type), addr, current_basic_block);
            }
            break;
        }
        case ConstInitVal::stringConst: {
            //TODO CHEK
            auto type_ptr = std::dynamic_pointer_cast<ArrayType>(symbol->get_type());
            auto size = type_ptr->get_size();
            auto int_vec = split_to_int(string_const);
            int i = 0;
            for (auto &it: int_vec) {
                auto addr = new GetElementPtrInstruction(value, new Constant(i, current_module->getContext()->getIntType()), current_basic_block);
                auto element_type = dynamic_cast<PointerTypePtr>(addr->get_value_return_type())->get_referenced_type();
                new StoreInstruction(new Constant(it, element_type), addr, current_basic_block);
                i++;
            }
            for (; i < size; ++i) {
                auto addr = new GetElementPtrInstruction(value, new Constant(i, current_module->getContext()->getIntType()), current_basic_block);
                auto element_type = dynamic_cast<PointerTypePtr>(addr->get_value_return_type())->get_referenced_type();
                new StoreInstruction(new Constant(0, element_type), addr, current_basic_block);
            }
            break;
        }
        default:
            throw std::logic_error("no initializer");
    }
}

void Visitor::visit_const_def(ConstDef &node,
    const std::shared_ptr<BasicType>& basic_type,
    bool isGlobal) {
    //1.TYPE DEFINE
    std::shared_ptr<SymType> type;
    if (node.const_expr) {
        std::shared_ptr<SymType> index_type;
        auto size = visit_add_exp(*node.const_expr->add_exp, index_type);
        assert(typeid(*size) == typeid(Constant));
        auto size_value = dynamic_cast<ConstantPtr>(size);
        type = std::make_shared<ArrayType>(size_value->get_value(), basic_type);
        //STORE THE SIZE
    }
    else {
        type = basic_type;
    }

    //2.MEMORY ALLOCATION
    const auto symbol = std::make_shared<Symbol>
    (type, node.identifier->getContent(),
        node.identifier->getLine(), 0);//ID?
    ValuePtr value = new Constant(0, current_module->getContext()->getIntType());
    if (current_sym_tab->add_symbol(symbol)) {
        //ALLOCA
        value = allocation(type, isGlobal, true, node.identifier->getContent());
    }
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    //3.INITIALIZATION
    if (!node.const_init_val) {
        return;
    }//no initializer
    std::string string = std::string();
    auto &string_const = node.const_init_val->string_const;
    if (string_const) {
        string = string_const->getContent();
    }
    if (isGlobal) {
        init_global_object(value, symbol, node.const_init_val->const_exps, string);
    }
    else {
        init_local_object(value, symbol, node.const_init_val->type, node.const_init_val->const_exps,
            string);
    }
}

void Visitor::visit_var_def(VarDef &node,
    const std::shared_ptr<BasicType>& basic_type,
    bool isGlobal) {
    //1.TYPE DEFINE
    std::shared_ptr<SymType> type;
    if (node.const_exp) {
        std::shared_ptr<SymType> index_type;
        auto size = visit_add_exp(*node.const_exp->add_exp, index_type);
        assert(typeid(*size) == typeid(Constant));
        auto size_value = dynamic_cast<ConstantPtr>(size);
        type = std::make_shared<ArrayType>(size_value->get_value(), basic_type);
    }
    else {
        type = basic_type;
    }

    //2.MEMORY ALLOCATION
    auto symbol = std::make_shared<Symbol>
    (type, node.identifier->getContent(), node.identifier->getLine(), 0);//ID? Line
    ValuePtr value = new Constant(0, current_module->getContext()->getIntType());
    if (current_sym_tab->add_symbol(symbol)) {
        value = allocation(type, isGlobal, false, node.identifier->getContent());
    }
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    //3.INITIALIZATION
    if (!node.init_val) {
        return;
    }
    std::string string = std::string();
    auto &string_const = node.init_val->string_const;
    if (string_const) {
        string = string_const->getContent();
    }
    if (isGlobal) {
        init_global_object(value, symbol, node.init_val->exps, string);
    }
    else {
        init_local_object(value, symbol, node.init_val->type, node.init_val->exps,
            string);
    }
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
    new ReturnInstruction(func->get_value_return_type(), func, ret_value, current_basic_block);
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

ValuePtr Visitor::visit_l_val_with_no_evaluate
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

            std::shared_ptr<SymType> index_type;
            auto offset = visit_add_exp(*node.exp->add_exp, index_type);

            auto pointer_val = symbol->get_addr();
            if (!pointer_val) {
                pointer_val = new LoadInstruction(symbol->get_addr_addr(), current_basic_block);
            }
            return new GetElementPtrInstruction(pointer_val, offset, current_basic_block);
            //ADDR OF ARRAY(IF GLOBAL, THE POINTER TYPE)
        }
        //the addr of a scalar symbol
        return symbol->get_addr();
    }

    errors.emplace_back('c', node.identifier->getLine());
    l_type = std::make_shared<BasicType>(false, BasicType::int_type);
    return nullptr;//todo check
}

ValuePtr Visitor::visit_l_val(LVal &node, std::shared_ptr<SymType> &type) {
    const auto symbol =
        current_sym_tab->get_from_all_scopes(node.identifier->getContent());

    if (symbol) {
        type = symbol->get_type()->evaluate();

        //if is array, return its element type
        if (typeid(*symbol->get_type()) == typeid(ArrayType)) {
            //FIRST OF ALL GET THE VALUE OF THE POINTER
            //IT CAN BE ADDR, OR STORED IN ADDR_ADDR
            auto pointer_val = symbol->get_addr();
            if (!pointer_val) {
                pointer_val = new LoadInstruction(symbol->get_addr_addr(), current_basic_block);
            }
            if (node.exp) {
                std::shared_ptr<SymType> index_type;
                auto offset = visit_add_exp(*node.exp->add_exp, index_type);
                auto array_type =
                    std::dynamic_pointer_cast<ArrayType>(symbol->get_type());
                type = array_type->element_type();

                //OBJECT * -> OBJECT_ELETYPE*
                auto addr = new GetElementPtrInstruction(pointer_val, offset, current_basic_block);
                return new LoadInstruction(addr, current_basic_block);
                //TODO DO NOT TYPE CONVERT HERE
                // return type_conversion(l_val->get_value_return_type(),
                //     current_module->getContext()->getIntType(), l_val);
                //on the right, load value
            }
            return new GetElementPtrInstruction(pointer_val,
                new Constant(0, current_module->getContext()->getIntType()), current_basic_block);
            //TODO 冗余
            //on the right, return the addr
        }

        //IF CONSTANT, CAL THE VALUE
        if (!symbol->get_addr()) {
            return symbol->get_value();
        }
        //IF NOT, LOAD THE VALUE
        return new LoadInstruction(symbol->get_addr(), current_basic_block);
        //TODO DO NOT TYPE CONVERT HERE
        // return type_conversion(l_val->get_value_return_type(),
        //     current_module->getContext()->getIntType(), l_val);
        //scalar type, load the value;
    }
    errors.emplace_back('c', node.identifier->getLine());
    type = std::make_shared<BasicType>(false, BasicType::int_type);
    return new Constant(0, current_module->getContext()->getIntType());
}

ValuePtr Visitor::visit_l_val_exp(LValExp &node) {
    if (std::holds_alternative<Exp>(node)) {
        auto exp = std::get_if<Exp>(&node);
        std::shared_ptr<SymType> type;
        return visit_add_exp(*exp->add_exp, type);
    }
    return new InputInstruction(current_module->getContext()->getIntType(), current_basic_block,
        std::holds_alternative<GetCharExp>(node));
}

void Visitor::visit_func(FuncDef &node) {
    //1. fill the sym table
    const auto return_type =
        std::make_shared<BasicType>(false, node.func_type->type->getType());
    const auto func_type =
        std::make_shared<FunctionType>(return_type);
    const auto func_ptr = std::make_shared<Symbol>
        (func_type, node.identifier->getContent(), node.line, 0);//TODO //SYMBOL
    if (current_sym_tab->add_symbol(func_ptr)) {}
    else {
        errors.emplace_back('b', node.identifier->getLine());
    }

    //2. new scope
    push_table();

    //3, define the func ir structure
    auto func = new Function(return_type->toValueType(current_module->getContext()),
        false, func_ptr->get_identifier());//VALUE
    func_ptr->insert_addr(func);
    new_basic_block(func);
    if (node.func_f_params) {
        visit_func_f_params(*node.func_f_params, func_ptr);
    }

    //4. visit block
    const auto func_return_info =
        func_type->evaluate()->isVoid() ? BLOCK_WITHOUT_RETURN : BLOCK_WITH_RETURN;
    visit_block(*node.block, func_return_info, false);

    current_basic_block->pad();

    //5. pop scope
    current_sym_tab = current_sym_tab->pop_scope();
}

void Visitor::visit_func_f_params
    (FuncFParams &node, const std::shared_ptr<Symbol>& symbol) {

    for (const auto &it: node.func_f_params) {
        //1. fill the sym table and get ValueRetType
        const auto basic_type =
            std::make_shared<BasicType>(false, it->b_type->getType());
        std::shared_ptr<SymType> type;
        ValueReturnTypePtr value_return_type;
        if (it->array_type) {
            type = std::make_shared<ArrayType>(0, basic_type);//TODO MAINTAIN 0, TO MARK IT IS A POINTER
            value_return_type = current_module->getContext()->getPointerType(basic_type->toValueType(
                current_module->getContext()));
        }
        else {
            type = basic_type;
            value_return_type = basic_type->toValueType(current_module->getContext());
        }

        //2. insert argument to function ir structure
        symbol->insert_parameter(type);
        auto arg = new Argument(value_return_type);
        auto allocation = new AllocaInstruction(value_return_type, current_basic_block);
        new StoreInstruction(arg, allocation, current_basic_block);
        current_basic_block->get_function()->insert_parameter(arg);

        //3. insert params to function symbol
        auto param_symbol = std::make_shared<Symbol>
        (type, it->identifier->getContent(), it->b_type->getLine(), 0);
        if (typeid(*dynamic_cast<PointerTypePtr>(allocation->get_value_return_type())
            ->get_referenced_type())
            == typeid(PointerType)) {
            param_symbol->insert_addr_addr(allocation);
        }
        else {
            param_symbol->insert_addr(allocation);
        }
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
            visit_decl(*decl, false);
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
    auto main_func = new Function(current_module->getContext()->getIntType(), true, "main");
    new_basic_block(main_func);
    visit_block(*node.block, BLOCK_WITH_RETURN, false);
    current_sym_tab = current_sym_tab->pop_scope();
}

std::string Visitor::get_string_name() {
    return "str_" + std::to_string(++str_id);
}


void Visitor::print_symbol(std::ostream &out) {
    for (const auto& table : sym_tables) {
        table->print_table(out);
    }
}

void Visitor::print_llvm(std::ostream &out) {
    current_module->print(out);
}