//
// Created by LENOVO on 2024/11/14.
//

#include "../../include/ir/instructions.h"

Instruction::Instruction(ValueReturnTypePtr return_type, ValueType type, BasicBlockPtr basic_block):
        User(return_type, type) {
    basic_block->add_inst(this);
}

void Instruction::mark_id(unsigned int &id_alloc) {
    if (id == 0 && get_value_return_type() != getContext()->getVoidType()) {
        id = id_alloc;
    }
    else {
        id_alloc--;
    }
}

BinaryInstruction::BinaryInstruction(ValueReturnTypePtr return_type, TokenType op,
    ValuePtr lhs, ValuePtr rhs, BasicBlockPtr basic_block):
    Instruction(return_type, ValueType::BinaryInst, basic_block) {
    try {this->op = tokentype_to_binary_op[op];} catch (std::invalid_argument& e) {std::cout << e.what() << '\n';}
    this->add_use(new Use(this, lhs));
    this->add_use(new Use(this, rhs));
    return_type->getContext()->SaveValue<BinaryInstruction>(this);
}

AllocaInstruction::AllocaInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block):
        Instruction(return_type->getContext()->getPointerType(return_type), ValueType::AllocaInst, basic_block) {
    return_type->getContext()->SaveValue<AllocaInstruction>(this);
    object_type = return_type;
}

ZextInstruction::ZextInstruction(ValuePtr value, BasicBlockPtr basic_block):
    Instruction(value->getContext()->getIntType(), ValueType::ZextInst, basic_block) {
    this->add_use(new Use(this, value));
}

TruncInstruction::TruncInstruction(ValuePtr value, BasicBlockPtr basic_block):
        Instruction(value->getContext()->getCharType(), ValueType::TruncInst, basic_block) {
    this->add_use(new Use(this, value));
}

void CallInstruction::print_full(std::ostream &out) {
    if (get_value_return_type() !=
        get_value_return_type()->getContext()->getVoidType()) {
        print(out);
        out << " = ";
        }
    out << "call ";
    get_value_return_type()->print(out);
    out << " @" << function->get_name() << "(";
    auto list = use_list;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (it != list.begin()) {
            out << ", ";
        }
        (*it)->getValue()->get_value_return_type()->print(out);
        out << " ";
        (*it)->getValue()->print(out);
    }
    out << ")";
}

void ReturnInstruction::print_full(std::ostream &out) {
    out << "ret ";
    if (get_value_return_type() !=
        get_value_return_type()->getContext()->getVoidType()) {
        auto value = use_list.at(0)->getValue();
        value->get_value_return_type()->print(out);
        out << " ";
        value->print(out);
        }
    else {
        out << "void";
    }
}

LoadInstruction::LoadInstruction(ValuePtr value, BasicBlockPtr basic_block):
    Instruction(dynamic_cast<PointerTypePtr>(value->get_value_return_type())->get_referenced_type(),
        ValueType::LoadInst, basic_block) {
    this->add_use(new Use(this, value));
}

StoreInstruction::StoreInstruction(ValuePtr value, ValuePtr addr, BasicBlockPtr basic_block):
        Instruction(value->get_value_return_type()->getContext()->getVoidType()
            , ValueType::StoreInst, basic_block) {
    this->add_use(new Use(this, value));
    this->add_use(new Use(this, addr));
}

OutputInstruction::OutputInstruction(ValuePtr value, BasicBlockPtr  basic_block):
        Instruction(value->get_value_return_type()->getContext()->getVoidType(),
            ValueType::OutputInst, basic_block), ch(false) {
    value_return_type = value->get_value_return_type();
    this->add_use(new Use(this, value));
}

OutputInstruction::OutputInstruction(ValuePtr value, BasicBlockPtr basic_block, bool ch):
        Instruction(value->get_value_return_type()->getContext()->getVoidType(),
            ValueType::OutputInst, basic_block), ch(ch) {
    value_return_type = value->get_value_return_type();
    this->add_use(new Use(this, value));
}


GetElementPtrInstruction::GetElementPtrInstruction(ValuePtr base, ValuePtr offset, BasicBlockPtr basic_block):
    Instruction(base->get_value_return_type()->getContext()
    ->getPointerType(dynamic_cast<PointerTypePtr>(base->get_value_return_type())
    ->get_referenced_type()->get_ele_type()), ValueType::GetElementInst, basic_block) {
    right_val = base->get_value_return_type();
    this->add_use(new Use(this, base));
    this->add_use(new Use(this, offset));
}
