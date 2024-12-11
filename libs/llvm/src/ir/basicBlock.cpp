//
// Created by LENOVO on 2024/11/14.
//
#include <algorithm>
#include <utility>

#include "../../include/ir/basicBlock.h"
#include "../../include/ir/constant.h"
#include "../../../include/configure.h"
#include "../../include/ir/function.h"


template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
    for (auto phi: phi_instructions) {
        phi.second->mark_id(id_alloc);
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
    for (auto phi:phi_instructions) {
        out << "\t";
        phi.second->print_full(out);
        out << std::endl;
    }
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
        std::visit(overloaded{
            [this, i](InstructionPtr &it) { it->mark_active(i); },
            [this, i](ArgumentPtr &it) { it->mark_active(i); },
        }
        ,it);
    }
    for (auto it: def_set) {
        std::visit(overloaded{
            [this, i](InstructionPtr &it) { it->mark_active(i); },
            [this, i](ArgumentPtr &it) { it->mark_active(i); },
        }
        ,it);
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

bool variantCompare(const std::variant<InstructionPtr, ArgumentPtr>& var,
    const InstructionPtr value) {
    if (std::holds_alternative<InstructionPtr>(var)) {
        return std::get<InstructionPtr>(var) == value;
    }
    return false;
}

void BasicBlock::fetch_cross(std::vector<Variable> &cross) {//TODO IS SLOW SHOULD PUT IN FUNCTION
    for (auto it: use_list) {
        auto inst = dynamic_cast<InstructionPtr>(it->getValue());
        if (inst && inst->active_block_seq.size() > 1) {
            std::set<Variable> vars;
            if (std::find_if(cross.begin(), cross.end(),
                [inst](const auto& v) { return variantCompare(v, inst);})
                == cross.end()) {
                cross.emplace_back(inst);
            }
            inst->is_global = true;
        }
    }
    for (auto it: phi_instructions) {
        cross.emplace_back(it.second);
        it.second->is_global = true;
    }
}

void BasicBlock::add_to_use_def(InstructionPtr inst) {
    if (typeid(*inst) == typeid(PCInstruction)) {
        auto pc = dynamic_cast<PCInstructionPtr>(inst);
        for (auto use: pc->get_use()) {
            if (def_set.find(use) == def_set.end()) {
                use_set.insert(use);
            }
        }
        for (auto def: pc->get_def()) {
            if (use_set.find(def) == use_set.end()) {
                def_set.insert(def);
            }
        }
        //DO NOT CAL AGAIN!!!
        return;
    }
    for (auto it: inst->use_list) {
        auto instruction_ptr = dynamic_cast<InstructionPtr>(it->getValue());
        if (instruction_ptr/*is a variable*/ && def_set.find(instruction_ptr) == def_set.end()) {
            use_set.insert(instruction_ptr);
        }
        else {
            auto arg = dynamic_cast<ArgumentPtr>(it->getValue());
            if (arg) {
                use_set.insert(arg);
            }
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

bool BasicBlock::empty_father() {
    return father_basic_blocks.empty();
}

void BasicBlock::set_dom_set(std::set<BasicBlockPtr> set) {
    strict_dom_set = std::move(set);
}

void BasicBlock::add_dominator(BasicBlockPtr ptr) {
    strict_dominators.insert(ptr);
}

void BasicBlock::cal_idom() {
    for (auto item : strict_dominators) {
        //all dominators
        //check which do not dominate dominators
        auto flag = 1;
        for (auto dominate: item->strict_dom_set) {
            if (strict_dominators.find(dominate) != strict_dominators.end()) {
                flag = 0;
            }
        }
        if (flag) {
            idom = item;
            item->direct_dom_set.insert(this);
            break;
        }
    }
}

void BasicBlock::add_DF_ele(BasicBlockPtr ptr) {
    DF_set.insert(ptr);
}

void BasicBlock::print_dir(std::ostream & ostream) {
    ostream << "dom_tree_child of ";
    print(ostream);
    ostream << ':';
    for (auto block: direct_dom_set) {
        block->print(ostream);
        ostream << ' ';
    }
}


void BasicBlock::print_df(std::ostream & ostream) {
    ostream << "df of ";
    print(ostream);
    ostream << ':';
    for (auto block: DF_set) {
        block->print(ostream);
        ostream << ' ';
    }
}

void BasicBlock::insert_phi_instruction(AllocaInstructionPtr alloca) {
    if (phi_instructions.find(alloca) == phi_instructions.end()) {
        auto phi = new PhiInstruction(alloca, this);
        phi_instructions[alloca] = phi;
    }
}

void BasicBlock::rename(std::map<AllocaInstructionPtr, std::vector<ValuePtr>> &defs) {
    std::map<AllocaInstructionPtr, ValuePtr> current_bb_def;
    for (auto phi:phi_instructions) {
        current_bb_def[phi.first] = phi.second;
    }
    auto it = use_list.begin();
    while (it != use_list.end()) {
        auto inst = dynamic_cast<InstructionPtr>((*it)->getValue());
        if (typeid(*inst) == typeid(LoadInstruction)) {
            auto load_inst = dynamic_cast<LoadInstructionPtr>(inst);
            auto addr = dynamic_cast<AllocaInstructionPtr>(load_inst->use_list.at(0)->getValue());
            if (addr) {
                if (current_bb_def.find(addr) != current_bb_def.end()) {
                    load_inst->substitute_instruction(current_bb_def[addr]);
                }
                else if (defs.find(addr) != defs.end()) {
                    load_inst->substitute_instruction(defs[addr].back());
                }
                else {
                    load_inst->substitute_instruction(new Constant(0,
                        addr->get_object_type()->getContext()->getIntType()));
                }
                it = use_list.erase(it);
            }
            else { it++; }
        }
        else if (typeid(*inst) == typeid(StoreInstruction)) {
            auto store = dynamic_cast<StoreInstructionPtr>(inst);
            auto addr = store->use_list.at(1)->getValue();
            if (typeid(*addr) == typeid(AllocaInstruction)) {
                auto alloca = dynamic_cast<AllocaInstruction*>(addr);
                auto data = store->use_list.at(0)->getValue();
                current_bb_def[alloca] = data;
                data->delete_user(store);
                it = use_list.erase(it);
            }
            else { it++; }
        }
        else if (typeid(*inst) == typeid(AllocaInstruction)) {
            auto alloca = dynamic_cast<AllocaInstructionPtr>(inst);
            if (alloca->get_object_type()->isScalar()) {
                it = use_list.erase(it);
            }
            else { it++; }
        }
        else { it++; }
    }
    //fill phi
    for (auto bb: goto_basic_blocks) {
        for (auto inst: bb->phi_instructions) {
            auto addr = inst.first;
            if (current_bb_def.find(addr) != current_bb_def.end()) {
                inst.second->add_option(current_bb_def[addr], this);
            }
            else if (defs.find(addr) != defs.end()) {
                inst.second->add_option(defs[addr].back(), this);
            }
            else {
                inst.second->add_option(new Constant(0,
                    addr->get_object_type()->getContext()->getIntType()), this);
            }
        }
    }

    //push chain stack
    for (auto item:current_bb_def) {
        if (defs.find(item.first) == defs.end()) {
            defs[item.first] = {};
        }
        defs[item.first].push_back(item.second);
    }
    //dsf
    for (auto bb: direct_dom_set) {
        bb->rename(defs);
    }
    //pop chain
    for (auto item:current_bb_def) {
        defs[item.first].pop_back();
        if (defs[item.first].empty()) {
            defs.erase(item.first);
        }
    }
}

void BasicBlock::add_inst_before_last(InstructionPtr inst) {
    assert(!use_list.empty());
    use_list.insert(use_list.end() - 1, new Use(this, inst));
}

void BasicBlock::substitute_goto(BasicBlockPtr old_bb, BasicBlockPtr new_bb) {
    for (auto & goto_basic_block : goto_basic_blocks) {
        if (goto_basic_block == old_bb) {
            goto_basic_block = new_bb;
        }
    }
    auto inst = use_list.back()->getValue();
    auto jump = dynamic_cast<JumpInstructionPtr>(inst);
    auto branch = dynamic_cast<BranchInstructionPtr>(inst);
    if (jump) {
        jump->substitute(old_bb, new_bb);
    }
    if (branch) {
        branch->substitute(old_bb, new_bb);
    }
}

void BasicBlock::delete_phi() {
    for (auto phi: phi_instructions) {
        auto inst = phi.second;
        auto map = inst->get_options();
        for (auto option: map) {
            auto pc = option.first->get_pc();
            if (!pc) {
                pc = new PCInstruction(option.first);
            }
            pc->add_edge(option.second, inst);
        }
    }
}