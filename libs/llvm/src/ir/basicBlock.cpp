//
// Created by LENOVO on 2024/11/14.
//

#include "../../include/ir/basicBlock.h"
#include "../../include/ir/constant.h"

BasicBlock::BasicBlock(ValueReturnTypePtr return_type, FunctionPtr function_ptr):
        User(return_type, ValueType::BasicBlock) {
    return_type->getContext()->SaveValue<BasicBlock>(this);
    function = function_ptr;
}

void BasicBlock::mark_id(unsigned int &id_alloc) {
    if (id == -1) {
        id = id_alloc;
        id_alloc++;
    }
    for (auto inst: use_list) {
        dynamic_cast<InstructionPtr>(inst->getValue())->mark_id(id_alloc);
        id_alloc++;
    }
}

void BasicBlock::print(std::ostream &out) {
    out << id;
}

void BasicBlock::print_full(std::ostream &out) {
    print(out);
    out << ":" << std::endl;
    for (auto inst: use_list) {
        out << "\t";
        dynamic_cast<InstructionPtr>(inst->getValue())->print_full(out);
        out << std::endl;
    }
}

void BasicBlock::pad() {
    if (use_list.empty()) {
        new ReturnInstruction(function->get_value_return_type(), function,
            new Constant(0, function->get_value_return_type()),
            this);
        return;
    }
    auto type_r = use_list.at(use_list.size() - 1)->getValue();
    if (typeid(*type_r) != typeid(ReturnInstruction) &&
        typeid(*type_r) != typeid(JumpInstruction) &&
        typeid(*type_r) != typeid(BranchInstruction)) {
        new ReturnInstruction(function->get_value_return_type(), function,
            new Constant(0, function->get_value_return_type()->getContext()->getIntType()),
            this);
    }
    //TODO JUMP
}

bool BasicBlock::enable_pad() {
    if (use_list.empty()) {
        return true;
    }
    auto type = use_list.at(use_list.size() - 1)->getValue();
    return typeid(*type) != typeid(ReturnInstruction) &&
        typeid(*type) != typeid(JumpInstruction) &&
        typeid(*type) != typeid(BranchInstruction);
}
