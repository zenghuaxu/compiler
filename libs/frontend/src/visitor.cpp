//
// Created by LENOVO on 2024/10/14.
//
#include <memory>

#include "../include/visitor.h"

#include "../../llvm/include/ir/module.h"
#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/function.h"
#include "../../llvm/include/ir/value.h"

/**
 *
 * @param origin origin type
 * @param target target type
 * @param value object need to alter type
 * @return instruction
 * NOTICE:: CHAR->INT ZERO EXTENSION?
 */
Visitor::Visitor(std::vector<Error> &errors): errors(errors) {
    current_sym_tab = std::make_shared<SymTable>();
    current_module = std::make_shared<Module>();
    current_basic_block = nullptr;
#ifdef SYMTABLE_
    sym_tables.push_back(current_sym_tab);
#endif
};

ValuePtr Visitor::type_conversion(ValueReturnTypePtr origin,
    ValueReturnTypePtr target, ValuePtr value) {
    if (origin != target) {
        if (typeid(*value) == typeid(Constant)) {
            return new Constant(dynamic_cast<ConstantPtr>(value)->get_value(), target);
        }
        //TODO CHECK OVERLOAD
        if (*origin < *target) {
            return new ZextInstruction(value, current_basic_block);
        }
        return new TruncInstruction(value, current_basic_block);
    }
    return value;
}

ModulePtr Visitor::visit(CompUnit &node) {
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
    return current_module;
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
    current_basic_block = new_basic_block(func);
    if (node.func_f_params) {
        visit_func_f_params(*node.func_f_params, func_ptr);
    }

    //4. visit block
    const auto func_return_info =
        func_type->evaluate()->isVoid() ? BLOCK_WITHOUT_RETURN : BLOCK_WITH_RETURN;
    visit_block(*node.block, func_return_info, false, nullptr, nullptr);

    func->pad();

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

void Visitor::visit_block(Block &node, int if_return, bool if_for, BasicBlockPtr break_return, BasicBlockPtr continue_return) {
    for (auto &item: node.block_items) {
        if (std::holds_alternative<Decl>(*item)) {
            const auto decl = std::get_if<Decl>(&*item);
            visit_decl(*decl, false);
        }
        else {
            const auto stmt = std::get_if<Stmt>(&*item);
            visit_stmt(*stmt, if_return ? BLOCK_PLAIN : BLOCK_WITHOUT_RETURN, if_for, break_return, continue_return);
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
    current_basic_block = new_basic_block(main_func);
    visit_block(*node.block, BLOCK_WITH_RETURN, false, nullptr,     nullptr);
    main_func->pad();
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

BasicBlockPtr Visitor::new_basic_block(FunctionPtr function) {
    auto block = new BasicBlock(current_module->getContext()->getVoidType(), function);
    function->insert_block(block);
    return block;
}
