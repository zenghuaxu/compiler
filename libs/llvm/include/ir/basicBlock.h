//
// Created by LENOVO on 2024/11/12.
//

#ifndef BASICBLOCK_H
#define BASICBLOCK_H
#include "../llvm.h"
#include "instructions.h"
#include "user.h"

class BasicBlock: public User{
    public:
    explicit BasicBlock(ValueReturnTypePtr return_type, FunctionPtr function_ptr);

    void mark_id(unsigned int &id_alloc);

    void print(std::ostream &out) override;

    void add_inst(InstructionPtr inst) {
        auto use = new Use(this, reinterpret_cast<ValuePtr>(inst));
        add_use(use);
    }

    void insert_goto(BasicBlockPtr basic_block) {
        goto_basic_blocks.push_back(basic_block);
        basic_block->insert_father(this);
    }

    FunctionPtr get_function() {
        return function;
    }

    ~BasicBlock() override = default;

    private:
    FunctionPtr function;
    std::vector<BasicBlockPtr> goto_basic_blocks;
    std::vector<BasicBlockPtr> father_basic_blocks;

    void insert_father(BasicBlockPtr basic_block) {
        father_basic_blocks.push_back(basic_block);
    }

};

#endif //BASICBLOCK_H
