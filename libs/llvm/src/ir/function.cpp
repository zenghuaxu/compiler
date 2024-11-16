//
// Created by LENOVO on 2024/11/14.
//

#include "../../include/ir/function.h"

Function::Function(ValueReturnTypePtr return_type, bool is_main,
    std::string name):
    Value(return_type, ValueType::Function) {
    if (is_main) {return_type->getContext()->SaveMainFunction(this);}
    else {return_type->getContext()->SaveFunction(this);}
    current_object_id = 0;
    this->name = std::move(name);
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
    for (auto block : blocks) {
        block->pad();
    }
}
