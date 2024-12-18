//
// Created by LENOVO on 2024/11/12.
//

#ifndef BASICBLOCK_H
#define BASICBLOCK_H
#include <set>
#include <unordered_set>

#include "../llvm.h"
#include "instructions.h"


class BasicBlock: public User{
    friend class Translator;
    friend class Function;
    public:
    explicit BasicBlock(ValueReturnTypePtr return_type, FunctionPtr function_ptr);

    void mark_id(unsigned int &id_alloc);

    void print(std::ostream &out) override;

    void print_full(std::ostream &out);

    void pad();
    bool enable_pad();

    void add_inst(InstructionPtr inst) {
        auto use = new Use(this, reinterpret_cast<ValuePtr>(inst));
        add_use(use);
    }

    void insert_goto(BasicBlockPtr basic_block) {
        goto_basic_blocks.push_back(basic_block);
        basic_block->insert_father(this);
    }

    void delete_goto(BasicBlockPtr basic_block) {
        for (int i = 0; i < goto_basic_blocks.size(); i++) {
            if (goto_basic_blocks[i] == basic_block) {
                goto_basic_blocks.erase(goto_basic_blocks.begin() + i);
            }
        }
    }

    void delete_father(BasicBlockPtr basic_block) {
        for (int i = 0; i < father_basic_blocks.size(); i++) {
            if (father_basic_blocks[i] == basic_block) {
                father_basic_blocks.erase(father_basic_blocks.begin() + i);
            }
        }
    }

    FunctionPtr get_function() {
        return function;
    }

    bool empty() {
        return use_list.empty();
    }

    // void merge(BasicBlockPtr basic_block) {
    //     for (auto father: father_basic_blocks) {
    //         father->delete_goto(this);
    //         father->insert_goto(basic_block);
    //     }
    //     delete this;
    // }

    std::vector<BasicBlockPtr> get_goto() {
        return goto_basic_blocks;
    }

    ~BasicBlock() override = default;

    void mark_active(int i);
    bool update_in_set();
    void fetch_cross(std::vector<Variable> &cross);
    void create_use_def();

    bool empty_father();

    void set_dom_set(std::set<BasicBlockPtr> set);
    std::set<BasicBlockPtr> get_strict_dom() { return strict_dom_set;}
    std::set<BasicBlockPtr> get_df() { return DF_set; }
    BasicBlockPtr get_idom() {return idom;}
    void add_dominator(BasicBlockPtr ptr);
    void cal_idom();
    void print_dir(std::ostream &ostream);
    void print_df(std::ostream &ostream);
    void add_DF_ele(BasicBlockPtr ptr);
    void insert_phi_instruction(AllocaInstructionPtr);
    void rename(std::map<AllocaInstructionPtr, std::vector<ValuePtr>> &defs);

    void add_inst_before_last(InstructionPtr inst);

    void substitute_goto(BasicBlockPtr old_bb, BasicBlockPtr new_bb);

    void delete_phi();

    void mark_pc(PCInstructionPtr pc_inst) { pc = pc_inst;}
    PCInstructionPtr get_pc() {return pc;}

    std::vector<UsePtr> get_use_list() {  return use_list; }

private:
    FunctionPtr function;
    std::vector<BasicBlockPtr> goto_basic_blocks;
    std::vector<BasicBlockPtr> father_basic_blocks;
    unsigned int id = -1;
    std::map<AllocaInstructionPtr, PhiInstructionPtr> phi_instructions;

    //寄存器分配：
    std::set<Variable> use_set;
    std::set<Variable> def_set;
    std::set<Variable> in_set;
    std::set<Variable> out_set;

    //支配集计算：
    BasicBlockPtr idom = nullptr;
    std::set<BasicBlockPtr> strict_dominators;
    std::set<BasicBlockPtr> strict_dom_set;
    std::set<BasicBlockPtr> direct_dom_set;
    std::set<BasicBlockPtr> DF_set;

    PCInstructionPtr pc = nullptr;

    void insert_father(BasicBlockPtr basic_block) {
        father_basic_blocks.push_back(basic_block);
    }

    void add_to_use_def(InstructionPtr inst);
};

#endif //BASICBLOCK_H
