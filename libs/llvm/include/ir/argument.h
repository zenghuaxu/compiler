//
// Created by LENOVO on 2024/11/12.
//

#ifndef ARGUMENT_H
#define ARGUMENT_H
#include "../llvm.h"
#include "value.h"

class Argument: public Value {
    public:
    explicit Argument(ValueReturnTypePtr return_type):
        Value(return_type, ValueType::Argument) {}

    ~Argument() override = default;
    void print(std::ostream &out) override {
        out << "%";
        out << id;
    }

    void set_id(unsigned int id) {
        this->id = id;
    }
    void mark_active(int i) {
        active_block_seq.insert(i);
    }
    std::set<int> get_active_block_seq() {
        return active_block_seq;
    }
    void add_conflict(Variable instruction) {
        conflicting_instructions.insert(instruction);
    }
    int get_conflict_count() {
        return conflicting_instructions.size();
    }
    bool contains_conflict(Variable instruction) {
        return conflicting_instructions.find(instruction) != conflicting_instructions.end();
    }
    private:
    FunctionPtr function;
    unsigned int id;
    std::set<int> active_block_seq;
    std::set<Variable> conflicting_instructions;
};

#endif //ARGUMENT_H
