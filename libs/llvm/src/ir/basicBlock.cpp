//
// Created by LENOVO on 2024/11/14.
//
#include <algorithm>

#include "../../include/ir/basicBlock.h"
#include "../../include/ir/constant.h"
#include "../../../include/configure.h"

BasicBlock::BasicBlock(ValueReturnTypePtr return_type, FunctionPtr function_ptr):
        User(return_type, ValueType::BasicBlock) {
    //return_type->getContext()->SaveValue<BasicBlock>(this);
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
        new ReturnInstruction(function->get_value_return_type()->getContext()->getVoidType(), function,
            new Constant(0, function->get_value_return_type()),
            this);
        return;
    }
    auto type_r = use_list.at(use_list.size() - 1)->getValue();
    if (typeid(*type_r) != typeid(ReturnInstruction) &&
        typeid(*type_r) != typeid(JumpInstruction) &&
        typeid(*type_r) != typeid(BranchInstruction)) {
        new ReturnInstruction(function->get_value_return_type()->getContext()->getVoidType(), function,
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

void BasicBlock::mark_active(int i) {
    for (auto it: in_set) {
// #ifdef MIPS_DEBUG
//         for (auto it: in_set) {
//             it->print_full(std::cout);
//             std::cout << " func:" << function->get_name() << " block: "  << id << std::endl;
//         }
// #endif
        it->mark_active(i);
    }
    for (auto it: def_set) {
        it->mark_active(i);
    }
}

bool BasicBlock::update_in_set() {
    auto size = in_set.size();
    //init out set
    for (auto block: goto_basic_blocks) {
        std::set_union(out_set.begin(), out_set.end(),
            block->in_set.begin(), block->in_set.end(),
            std::inserter(out_set, out_set.end()));
    }
    //-def
    std::set_difference(out_set.begin(), out_set.end(),
        def_set.begin(), def_set.end(),
        std::inserter(in_set, in_set.end()));
    //并上use
    std::set_union(use_set.begin(), use_set.end(),
        in_set.begin(), in_set.end(),
        std::inserter(in_set, in_set.end()));
    return in_set.size() != size;
}

void BasicBlock::fetch_cross(std::vector<InstructionPtr> &cross) {//TODO IS SLOW SHOULD PUT IN FUNCTION
    for (auto it: use_list) {
        auto inst = dynamic_cast<InstructionPtr>(it->getValue());
        if (inst && inst->active_block_seq.size() > 1) {
            cross.emplace_back(inst);
            inst->is_global = true;
        }
    }
}

void BasicBlock::add_to_use_def(InstructionPtr inst) {
    for (auto it: inst->use_list) {
        auto instruction_ptr = dynamic_cast<InstructionPtr>(it->getValue());
        if (instruction_ptr/*is a variable*/ && def_set.find(instruction_ptr) == def_set.end()) {
            use_set.insert(instruction_ptr);
        }
    }
    if (use_set.find(inst) == use_set.end()) {
        def_set.insert(inst);
    }
}

void BasicBlock::create_use_def() {
    for (auto it: use_list) {
        add_to_use_def(dynamic_cast<InstructionPtr>(it->getValue()));
    }
}

