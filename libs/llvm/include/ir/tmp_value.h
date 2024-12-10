//
// Created by LENOVO on 2024/12/10.
//

#ifndef TMP_VALUE_H
#define TMP_VALUE_H
#include "value.h"

#endif //TMP_VALUE_H

class TmpValue : public Instruction {
    public:
    TmpValue(): Instruction(nullptr, ValueType::tmp, nullptr) {}
    void print(std::ostream &out) override {
        out << "TmpValue";
    }
    void print_full(std::ostream &out) override {
        out << "TmpValue: should not appear";
    }
};