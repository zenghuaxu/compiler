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

    private:
    FunctionPtr function;
    unsigned int id;
};

#endif //ARGUMENT_H
