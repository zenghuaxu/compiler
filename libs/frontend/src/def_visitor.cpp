//
// Created by LENOVO on 2024/11/24.
//
#include "../include/visitor.h"

#include "../../llvm/include/ir/module.h"
#include "../../llvm/include/ir/globalValue.h"
#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/instructions.h"

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
            return; //const scalar
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
    else {
        symbol->insert_value(new Constant(0, symbol->get_type()
            ->toValueType(current_module->getContext())));
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
                if (typeid(*init) != typeid(Constant)) {
                    symbol->insert_value(new Constant(0, current_module->getContext()->getIntType()));
                }
                //assert(typeid(*init) == typeid(Constant));
                else {
                    auto constant = dynamic_cast<ConstantPtr>(init);
                    symbol->insert_value(constant);
                }
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
        return;
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
        return;
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
