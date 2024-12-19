//
// Created by LENOVO on 2024/11/14.
//
#include "../../../mips/include/mipsReg.h"
#include "../../include/ir/function.h"
#include "../../../include/configure.h"
#include "../../../mips/include/mips.h"

#include <algorithm>
#include <vector>

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Function::Function(ValueReturnTypePtr return_type, bool is_main,
                   std::string name):
    Value(return_type, ValueType::Function) {
    if (is_main) {return_type->getContext()->SaveMainFunction(this);}
    else {return_type->getContext()->SaveFunction(this);}
    current_object_id = 0;
    this->name = std::move(name);
    blocks.push_back(new BasicBlock(return_type->getContext()->getVoidType(), this));
}

void Function::insert_allocation(InstructionPtr alloc) {
    blocks.at(0)->add_inst(alloc);
}

void Function::mark_id() {
    for (const auto bb: blocks) {
        bb->mark_id(current_object_id);
    }
}

void Function::print(std::ostream &out) {
    out << std::endl << "define dso_local ";
    print_value_return_type(out);
    out << " @" + name + "(";
    for (int i = 0; i < args.size(); i++) {
        args.at(i)->get_value_return_type()->print(out);
        out << " ";
        args.at(i)->print(out);
        if (i != args.size() - 1) {
            out << ", ";
        }
    }
    out << ") {" << std::endl;

    mark_id();
    for (auto & block : blocks) {
        block->print_full(out);
    }
    out << "}" << std::endl;
}

void Function::pad() {//TODO INEFFICIENT
    blocks.at(0)->insert_goto(blocks.at(1));
    new JumpInstruction(blocks.at(1), blocks.at(0));
    for (auto block : blocks) {
        block->pad();
    }
}

void Function::create_conflict_graph() {
    //活跃变量分析
    for (auto & block : blocks) {
        block->create_use_def();
    }

    bool flag = true;
    while (flag) {
        flag = false;
        for (auto block:blocks) {
            flag = flag || block->update_in_set();
        }
    }

    int i = 0;
    for (auto block:blocks) {
        block->mark_active(++i);
    }

    //冲突图
    for (auto block:blocks) {
        block->fetch_cross(cross_block_variable);
    }
    for (auto arg: args) {
        if (!arg->get_active_block_seq().empty()) {
            cross_block_variable.emplace_back(arg);
        }
    }

    #ifdef MIPS_DEBUG
    for (auto it: cross_block_variable) {
        std::cout << "cross_active in func:" << name << ": ";
        std::visit(overloaded{
            [this](InstructionPtr &inst) {inst->print(std::cout);},
            [this](ArgumentPtr &arg) {arg->print(std::cout);},
        }, it);
        std::cout << std::endl;
    }
    #endif
    i = 0;
    for (; i < cross_block_variable.size(); i++) {
        for (int j = i + 1; j < cross_block_variable.size(); j++) {
            auto i_object = cross_block_variable.at(i);
            auto j_object = cross_block_variable.at(j);
            auto front = get_active_blocks(i_object);
            auto back  = get_active_blocks(j_object);
            std::set<int> inter;
            std::set_intersection(front.begin(), front.end(), back.begin(), back.end(),
                std::inserter(inter, inter.end()));
            if (!inter.empty()) {
                add_conflict(i_object, j_object);
                add_conflict(j_object, i_object);
                #ifdef MIPS_DEBUG
                std::visit(overloaded{
                    [this](InstructionPtr &inst) {inst->print(std::cout);},
                        [this](ArgumentPtr &arg) {arg->print(std::cout);},
                }, i_object);
                std::cout << " ";
                std::visit(overloaded{
                    [this](InstructionPtr &inst) {inst->print(std::cout);},
                        [this](ArgumentPtr &arg) {arg->print(std::cout);},
                }, j_object);
                std::cout << std::endl;
                #endif
            }
        }
    }
}

std::set<int> Function::get_active_blocks(Variable var) {
    return std::visit(overloaded{
    [this](InstructionPtr &inst) { return inst->get_active_block_seq();},
    [this](ArgumentPtr &arg) { return arg->get_active_block_seq();},}
    ,var);
}

void Function::add_conflict(Variable i, Variable j) {
    std::visit(overloaded{
        [this, j](InstructionPtr &inst) {inst->add_conflict(j);},
            [this, j](ArgumentPtr &arg) {arg->add_conflict(j);},
    }, i);
}

//图着色
void Function::global_register_map(std::vector<SaveRegPtr> &save_regs,
    std::unordered_map<ValuePtr, RegPtr> &map, DynamicOffsetPtr offset) {
    std::vector<Variable> pop_values;
    std::set<Variable> un_map;
    std::set<Variable> mapped;
    std::unordered_map<Variable, int> conflict_egdes;

    if (cross_block_variable.empty()) {
        return;
    }

    //init
    for (auto it: cross_block_variable) {
        conflict_egdes[it] = std::visit(overloaded{
            [this](InstructionPtr &inst) { return inst->get_conflict_count();},
                [this](ArgumentPtr &arg) { return arg->get_conflict_count();}
        },it);
    }

    //pop
    while (conflict_egdes.size() > 1) {
        //find the edge
        bool alloc_success = false;
        for (auto it: conflict_egdes) {
            if (conflict_egdes.size() > 1 && it.second < save_regs.size()) {
                pop_values.push_back(it.first);
                conflict_egdes.erase(it.first);
                for (auto rest: conflict_egdes) {
                    if (var_contains_conflict(rest.first, it.first)) {
                        conflict_egdes.at(rest.first)--;
                    }
                }
                alloc_success = true;
                break;
            }
        }

        if (!alloc_success) {
            auto it = conflict_egdes.begin();
            un_map.insert(conflict_egdes.begin()->first);
            conflict_egdes.erase(conflict_egdes.begin());
            for (auto rest: conflict_egdes) {
                if (var_contains_conflict(rest.first, it->first)) {
                    conflict_egdes.at(rest.first)--;
                }
            }
        }
    }

    map[get_key(conflict_egdes.begin()->first)] = save_regs.at(0);
    mapped.insert(conflict_egdes.begin()->first);
    int max = 1;
    //push
    while (!pop_values.empty()) {
        auto node = pop_values.at(pop_values.size() - 1);
        pop_values.pop_back();

        std::set<int> colors = {};
        for(auto it: mapped) {
            if (var_contains_conflict(node, it)) {
                colors.insert(map.at(get_key(it))->get_id());
            }
        }

        for (int i = 0; i < SAVE_NUM; i++) { //*
            if (colors.find(i) == colors.end()) {
                map[get_key(node)] = save_regs.at(i);
                mapped.insert(node);
                max = std::max(max, i + 1);
                break;
            }
        }
    }
    saved_reg_used_num = max;//NOTICE : OK AS LONG AS THE STRATEGY OF * REMAINS

    //cope with mem
    //TODO CHECK ALLOC
    for (auto it: un_map) {
        MemOffsetPtr mem_offset;
        if (get_key(it)->get_value_return_type()->get_ele_type() ==
            get_key(it)->get_value_return_type()->getContext()->getCharType()) {
            mem_offset = new MemOffset(offset, get_key(it)->get_value_return_type()->get_byte_size(), 1);
        }
        else {
            mem_offset = new MemOffset(offset, get_key(it)->get_value_return_type()->get_byte_size(), 4);
        }
        map[get_key(it)] = mem_offset;
    }

#ifdef MIPS_DEBUG
    for (auto it: map) {
        auto value = it.first;
        value->print(std::cout);
        std::cout << " in func " << name <<" map to save" << it.second->get_id() << std::endl;
    }
#endif
}

ValuePtr Function::get_key(Variable var) {
    return std::visit(overloaded{
    [this](InstructionPtr &inst) {return dynamic_cast<ValuePtr>(inst);},
        [this](ArgumentPtr &arg) {
            return dynamic_cast<ValuePtr>(arg);},} ,var);
}

bool Function::var_contains_conflict(Variable var, Variable other) {
    return std::visit(overloaded{
    [this, other](InstructionPtr &inst) { return inst->contains_conflict(other);},
        [this, other](ArgumentPtr &arg) { return arg->contains_conflict(other);}},
            var);
}

void Function::emit_blocks() {
    std::set<BasicBlockPtr> all;
    for (auto item: blocks) {
        all.insert(item);
    }
    std::set<BasicBlockPtr> reachable;
    dfs(blocks.at(0), nullptr, reachable);
    std::set<BasicBlockPtr> emit;
    std::set_difference(all.begin(), all.end(),
        reachable.begin(), reachable.end(),
        std::inserter(emit, emit.begin()));
    for (auto it: emit) {
        auto emitted = std::find(blocks.begin(), blocks.end(), it);
        if (emitted != blocks.end()) {
            blocks.erase(emitted);
        }
    }
}

void Function::create_dom() {
    std::set<BasicBlockPtr> all;
    for (auto item: blocks) {
        all.insert(item);
    }
    for (auto & block : blocks) {
        auto prohibited = block;
        std::set<BasicBlockPtr> reachable, dom;
        dfs(blocks.at(0), prohibited, reachable);
        std::set_difference(all.begin(), all.end(),
            reachable.begin(), reachable.end(),
            std::inserter(dom, dom.end()));
        dom.erase(block);
        block->set_dom_set(dom);
        for (auto item:dom) {
            item->add_dominator(block);
        }
        //支配和被支配
        ///debug new
        std::cout<< "dom of ";
        block->print(std::cout);
        std::cout << ":";
        for (auto item:dom) {
            item->print(std::cout);
            std::cout << " ";
        }
        std::cout << std::endl;
    }
}

void Function::dfs(BasicBlockPtr root, BasicBlockPtr prohibited,
    std::set<BasicBlockPtr>& reachable) {
    if (root == prohibited) {
        return;
    }
    reachable.insert(root);
    for (auto item: root->get_goto()) {
        if (reachable.find(item) == reachable.end()) {
            dfs(item, prohibited, reachable);
        }
    }
}

void Function::create_dom_tree() {
    for (auto block:blocks) {
        block->cal_idom();
    }
    ///debug new
    for (auto block:blocks) {
        block->print_dir(std::cout);
        std::cout << std::endl;
    }
}

void Function::cal_DF() {
    for (auto block:blocks) {
        // all (a,b)
        for(auto b : block->get_goto()) {
            auto x = block;
            auto set = x->get_strict_dom();
            while (x && set.find(b) == set.end()) {
                //x not strictly dominate b
                x->add_DF_ele(b);
                x = x->get_idom();
                if (x) {
                    set = x->get_strict_dom();
                }
            }
        }
    }
    ///debug new
    for (auto block:blocks) {
        block->print_df(std::cout);
        std::cout << std::endl;
    }
}

void Function::insert_phi() {
    auto var_block = blocks.at(0);
    for (auto use : var_block->get_use_list()) {
        auto inst = dynamic_cast<AllocaInstructionPtr>(use->getValue());
        if (inst && inst->get_object_type()->isScalar()) {
            std::set<BasicBlockPtr> F_phi_added;
            std::vector<BasicBlockPtr> W_defs;
            for (auto user:inst->get_user_list()) {
                if (typeid(*user) == typeid(StoreInstruction)) {
                    auto store_inst = dynamic_cast<StoreInstructionPtr>(user);
                    W_defs.push_back(store_inst->get_basic_block());
                }
            }
            for(int i = 0; i < W_defs.size(); i++) {
                for (auto bb: W_defs.at(i)->get_df()) {
                    if (F_phi_added.find(bb) == F_phi_added.end()) {
                        bb->insert_phi_instruction(inst);
                        F_phi_added.insert(bb);
                        if (std::find(W_defs.begin(), W_defs.end(), bb) == W_defs.end()) {
                            W_defs.push_back(bb);
                        }
                    }
                }
            }
        }
        //mem2reg
    }
}

void Function::rename() {
    std::map<AllocaInstructionPtr, std::vector<ValuePtr>> defs;
    blocks.at(0)->rename(defs);
}


void Function::delete_phi() {
    //remove critical edge
    for (auto block:blocks) {
        if (block->get_goto().size() > 1) {
            for (auto bb: block->get_goto()) {
                if (bb->father_basic_blocks.size() > 1 &&
                    !bb->phi_instructions.empty()) {
                    auto new_bb = new BasicBlock(bb->getContext()->getVoidType(), this);
                    this->insert_block(new_bb);
                    //换jump/branch
                    block->substitute_goto(bb, new_bb);
                    new JumpInstruction(bb, new_bb);
                    bb->delete_father(block);
                }
            }
        }
    }
    //change phi to pc
    for (auto block:blocks) {
        block->delete_phi();
    }
}

void Function::mark_active_for_dce() {
    for (auto block:blocks) {
        for (auto use : block->get_use_list()) {
            auto inst = dynamic_cast<InstructionPtr>(use->getValue());
            if (inst) {
                if (typeid(*inst) == typeid(StoreInstruction) ||
                    typeid(*inst) == typeid(BranchInstruction) ||
                    typeid(*inst) == typeid(JumpInstruction) ||
                    typeid(*inst) == typeid(CallInstruction) ||
                    typeid(*inst) == typeid(ReturnInstruction) ||
                    typeid(*inst) == typeid(InputInstruction) ||
                    typeid(*inst) == typeid(OutputInstruction)) {
                    put_useful(inst);
                }
            }
        }
    }
    for (auto inst:useful) {
        inst->mark_active_for_dce();
    }
}

void Function::dce() {
    for (auto block:blocks) {
        block->dce();
    }
}

void Function::lvn() {
    for (auto block:blocks) {
        block->LVN();
    }
}
