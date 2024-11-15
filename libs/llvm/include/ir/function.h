//
// Created by LENOVO on 2024/11/12.
//

#ifndef FUNCTION_H
#define FUNCTION_H
#include <utility>
#include <iostream>

#include "../llvm.h"
#include "basicBlock.h"
#include "user.h"
#include "argument.h"
#include "instructions.h"

class Function:public Value{
    public:
    explicit Function(ValueReturnTypePtr return_type, bool is_main,
    std::string name);

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

    std::string get_name() {
        return name;
    }

    ValueReturnTypePtr get_arg_return_type(int index) {
        return args.at(index)->get_value_return_type();
    }

    private:
    std::vector<ArgumentPtr> args;
    std::vector<BasicBlockPtr> blocks;
    std::string name;
    unsigned int current_object_id;
};

#endif //FUNCTION_H
