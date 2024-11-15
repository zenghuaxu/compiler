//
// Created by LENOVO on 2024/11/14.
//

#include "../../include/ir/basicBlock.h"

BasicBlock::BasicBlock(ValueReturnTypePtr return_type, FunctionPtr function_ptr):
        User(return_type, ValueType::BasicBlock) {
    return_type->getContext()->SaveValue<BasicBlock>(this);
    function = function_ptr;
}

void BasicBlock::mark_id(unsigned int &id_alloc) {
    for (auto inst: use_list) {
        ++id_alloc;
        dynamic_cast<InstructionPtr>(inst->getValue())->mark_id(id_alloc);
    }
}

void BasicBlock::print(std::ostream &out) {
    for (auto inst: use_list) {
        out << "\t";
        dynamic_cast<InstructionPtr>(inst->getValue())->print_full(out);
        out << std::endl;
    }
}
