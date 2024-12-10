//
// Created by LENOVO on 2024/11/24.
//
#include "../include/visitor.h"
#include "../../include/configure.h"

#include "../../llvm/include/llvm.h"
#include "../../llvm/include/ir/module.h"
#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/instructions.h"
#include "../../llvm/include/ir/function.h"
#include "../../llvm/include/ir/value.h"

ValuePtr Visitor::visit_number(Number &node) const {
    auto content = node.token->getContent();
    int num = std::stoi(content);
    return new Constant(num, current_module->getContext()->getIntType());
}

ValuePtr Visitor::visit_character(Character &node) const {
    node.token->getContent();
    auto ch = node.get_value();
    return new Constant(ch, current_module->getContext()->getIntType());
}

void Visitor::visit_l_or_exp(LOrExp &node, BasicBlockPtr true_block, BasicBlockPtr false_block) {
    if (!node.lhs) {
        visit_l_and_exp(*node.rhs, true_block, false_block);
        return;
    }
    auto rhs_bb = new_basic_block(current_basic_block->get_function());
    visit_l_or_exp(*node.lhs, true_block, rhs_bb);
    current_basic_block = rhs_bb;
    visit_l_and_exp(*node.rhs, true_block, false_block);
}

void Visitor::visit_l_and_exp(LAndExp &node, BasicBlockPtr true_block, BasicBlockPtr false_block) {
    if (!node.lhs) {
        auto condition = visit_eq_exp(*node.rhs);

        if (typeid(*condition->get_value_return_type()) != typeid(BitType)) {
            condition = new CompareInstruction(current_module->getContext()->getBitType(), TokenType::NEQ,
                condition, new Constant(0, condition->get_value_return_type()),
                current_basic_block);
        } //to bittype
        new BranchInstruction(condition, true_block, false_block, current_basic_block);
        return;
    }
    auto rhs_bb = new_basic_block(current_basic_block->get_function());
    visit_l_and_exp(*node.lhs, rhs_bb, false_block);
    current_basic_block = rhs_bb;
    auto condition = visit_eq_exp(*node.rhs);
    if (typeid(*condition->get_value_return_type()) != typeid(BitType)) {
        condition = new CompareInstruction(current_module->getContext()->getBitType(), TokenType::NEQ,
            condition, new Constant(0, condition->get_value_return_type()),
            current_basic_block);
    } //to bittype
    new BranchInstruction(condition, true_block, false_block, current_basic_block);
}

ValuePtr Visitor::visit_eq_exp(EqExp &node) {
    if (!node.lhs) {
        return visit_rel_exp(*node.rhs);
    }
    auto left = visit_eq_exp(*node.lhs);
    auto right = visit_rel_exp(*node.rhs);

    left = type_conversion(left->get_value_return_type(),
    current_module->getContext()->getIntType(), left);
    right = type_conversion(right->get_value_return_type(),
        current_module->getContext()->getIntType(), right);
    return new CompareInstruction(current_module->getContext()->getBitType(),
        node.op->getType(), left, right, current_basic_block);
}

ValuePtr Visitor::visit_rel_exp(RelExp &node) {
    std::shared_ptr<SymType> type;
    if (!node.lhs) {
        return visit_add_exp(*node.rhs, type);
    }
    auto left = visit_rel_exp(*node.lhs);
    auto right = visit_add_exp(*node.rhs, type);

    left = type_conversion(left->get_value_return_type(),
        current_module->getContext()->getIntType(), left);
    right = type_conversion(right->get_value_return_type(),
        current_module->getContext()->getIntType(), right);
    return new CompareInstruction(current_module->getContext()->getBitType(),
        node.op->getType(), left, right, current_basic_block);
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
        return call_value;
    }
    if (std::holds_alternative<OpUnaryExp>(node)) {
        const auto unop = std::get_if<OpUnaryExp>(&node);
        auto unary = visit_unary_exp(*unop->unary_exp, type);
        if (typeid(*unary) == typeid(Constant)) {
            auto value = dynamic_cast<ConstantPtr>(unary)->get_value();
            if ((*unop->unary_op->op_token) == TokenType::MINU) {
                return new Constant(-value, current_module->getContext()->getIntType());
            }
            if ((*unop->unary_op->op_token) == TokenType::NOT) {
                return new Constant(!value, current_module->getContext()->getIntType());
            }
            return unary;
        }
        auto token = *unop->unary_op->op_token;
        if (!(token == TokenType::PLUS)) {
            unary = type_conversion(unary->get_value_return_type(),
                current_module->getContext()->getIntType(), unary);
            return new UnaryOpInstruction(token == TokenType::MINU ? current_module->getContext()->getIntType() : current_module->getContext()->getBitType(),
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
                auto call =  new CallInstruction(return_type,
                    dynamic_cast<FunctionPtr>(symbol->get_addr()), current_basic_block);
                auto new_block = new_basic_block(current_basic_block->get_function());
                current_basic_block->insert_goto(new_block);
                new JumpInstruction(new_block, current_basic_block);
                current_basic_block = new_block;
                return call;
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
            auto new_block = new_basic_block(current_basic_block->get_function());
            current_basic_block->insert_goto(new_block);
            new JumpInstruction(new_block, current_basic_block);
            current_basic_block = new_block;
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
