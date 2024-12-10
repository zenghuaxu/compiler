//
// Created by LENOVO on 2024/11/6.
//

#ifndef CONSTANT_H
#define CONSTANT_H

#include <cstdint>

#include "../llvm.h"
#include"value.h"
#include "../valueReturnType.h"

enum class ConstantOp {
    ADD, SUB, MUL, DIV, MOD, 
};

inline std::unordered_map<TokenType, ConstantOp> tokentype_to_const_op = {
    {TokenType::PLUS, ConstantOp::ADD},
    {TokenType::MINU, ConstantOp::SUB},
    {TokenType::MULT, ConstantOp::MUL},
    {TokenType::DIV, ConstantOp::DIV},
    {TokenType::MOD, ConstantOp::MOD},
};

class Constant : public Value {

    public:
    Constant(int const_value, ValueReturnTypePtr return_type):
        Value(return_type, ValueType::Constant) {
        this->int_va =  const_value;
        return_type->getContext()->SaveValue(this);
    };

    ~Constant() override {
        getContext()->DeleteValue(this);
    }

    void print(std::ostream &out) override {
        // get_value_return_type()->print(out);
        // out << " ";
        out << int_va;
    }

    [[nodiscard]] int get_value() const {
        return this->int_va;
    }

    static ConstantPtr merge(ConstantPtr left, ConstantPtr right, TokenType op_type) {
        auto constant =  new Constant(calculate(left, right, op_type), left->getContext()->getIntType());
        return constant;
    }

    static int calculate (ConstantPtr left, ConstantPtr right, TokenType op_type) {
        auto op = tokentype_to_const_op[op_type];
        switch (op) {
            case ConstantOp::ADD: return left->get_value() + right->get_value();
            case ConstantOp::SUB: return left->get_value() - right->get_value();
            case ConstantOp::MUL: return left->get_value() * right->get_value();
            case ConstantOp::DIV: return left->get_value() / right->get_value();
            case ConstantOp::MOD: return left->get_value() % right->get_value();
        }

        throw std::invalid_argument("Invalid constant op");
    }

    private:
    //int8_t char_va;
    int32_t int_va;
};

#endif //CONSTANT_H
