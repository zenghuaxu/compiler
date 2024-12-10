//
// Created by LENOVO on 2024/11/12.
//

#ifndef FUNCTION_H
#define FUNCTION_H
#include <unordered_map>

#include "../llvm.h"
#include "basicBlock.h"
#include "argument.h"
#include "instructions.h"
#include "../../../mips/include/mips.h"

class Function:public Value{
    friend class Translator;
    public:
    explicit Function(ValueReturnTypePtr return_type, bool is_main,
    std::string name);

    void insert_allocation(InstructionPtr alloc);

    ~Function() override {
        for (const auto arg : args) {
            delete arg;
        }
    }

    void insert_parameter(ArgumentPtr arg) {
        args.push_back(arg);
        arg->set_id(current_object_id++);
    }

    void insert_block(BasicBlockPtr block) {
        blocks.push_back(block);
    }

    void mark_id();

    void print(std::ostream &out) override;

    void pad();
    void create_conflict_graph();

    std::set<int> get_active_blocks(Variable var);

    void add_conflict(Variable i, Variable j);

    void global_register_map(std::vector<SaveRegPtr> &save_regs, std::unordered_map<ValuePtr, RegPtr> &map,
                             DynamicOffsetPtr offset);

    ValuePtr get_key(Variable var);

    bool var_contains_conflict(Variable var, Variable other);

    void emit_blocks();

    void create_dom();

    void dfs(BasicBlockPtr root, BasicBlockPtr prohibited, std::set<BasicBlockPtr> &reachable);

    void create_dom_tree();

    void cal_DF();

    void insert_phi();

    void rename();

    void print_dom(std::ostream &os);

    std::string get_name() {
        return name;
    }

    ValueReturnTypePtr get_arg_return_type(int index) {
        return args.at(index)->get_value_return_type();
    }

    void delete_bb(BasicBlockPtr block) {
        for (int i = 0; i < blocks.size(); i++) {
            if (blocks[i] == block) {
                blocks.erase(blocks.begin() + i);
            }
        }
    }

    BasicBlockPtr get_first_bb() {
        return blocks.at(0);
    }
    void delete_phi();

    private:
    std::vector<ArgumentPtr> args;
    std::vector<BasicBlockPtr> blocks;
    std::string name;
    unsigned int current_object_id;
    std::vector<std::set<BasicBlockPtr>> strict_dom_sets;

    std::vector<InstructionPtr> cross_block_variable;
    int saved_reg_used_num = 0;
};

#endif //FUNCTION_H
