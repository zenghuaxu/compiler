//
// Created by LENOVO on 2024/11/14.
//

#include "../../include/ir/instructions.h"
#include "../../include/ir/basicBlock.h"
#include "../../include/ir/function.h"

Instruction::Instruction(ValueReturnTypePtr return_type, ValueType type, BasicBlockPtr basic_block):
        User(return_type, type) {
    if (type == ValueType::tmp) {
        return;
    }
    if (type == ValueType::AllocaInst) {
        basic_block->get_function()->insert_allocation(this);
        this->basic_block = basic_block->get_function()->get_first_bb();
    }
    else if (type == ValueType::PCInst) {
        basic_block->add_inst_before_last(this);
        this->basic_block = basic_block;
    }
    else if (type != ValueType::PhiInst) {
        basic_block->add_inst(this);
        this->basic_block = basic_block;
    }
    return_type->getContext()->SaveValue(this);
}

void Instruction::mark_id(unsigned int &id_alloc) {
    if (id == 0 && get_value_return_type() != getContext()->getVoidType()) {
        id = id_alloc;
    }
    else {
        id_alloc--;
    }
}

UnaryOpInstruction::UnaryOpInstruction(ValueReturnTypePtr return_type, TokenType op,
    ValuePtr value, BasicBlockPtr basic_block):
    Instruction(return_type, ValueType::UnaryInst, basic_block) {
    try {this->op = tokentype_to_unary_op[op];} catch(std::invalid_argument& e) {std::cout << e.what() << '\n';}
    add_use(new Use(this, value));
}

BinaryInstruction::BinaryInstruction(ValueReturnTypePtr return_type, TokenType op,
    ValuePtr lhs, ValuePtr rhs, BasicBlockPtr basic_block):
    Instruction(return_type, ValueType::BinaryInst, basic_block) {
    try {this->op = tokentype_to_binary_op[op];} catch (std::invalid_argument& e) {std::cout << e.what() << '\n';}
    this->add_use(new Use(this, lhs));
    this->add_use(new Use(this, rhs));
}

CompareInstruction::CompareInstruction(ValueReturnTypePtr return_type, TokenType op,
    ValuePtr lhs, ValuePtr rhs, BasicBlockPtr basic_block):
    Instruction(return_type, ValueType::CompareInst, basic_block) {
    try {this->op = tokentype_to_comp_op[op];} catch (std::invalid_argument& e) {std::cout << e.what() << '\n';}
    this->add_use(new Use(this, lhs));
    this->add_use(new Use(this, rhs));
    comp_type = lhs->get_value_return_type();
}

BranchInstruction::BranchInstruction(ValuePtr condition, BasicBlockPtr true_block, BasicBlockPtr false_block,
    BasicBlockPtr current_block):
    Instruction(condition->get_value_return_type()->getContext()->getVoidType(),
        ValueType::BranchInst, current_block) {
    this->add_use(new Use(this, condition)); //0
    this->add_use(new Use(this, true_block)); //1
    this->add_use(new Use(this, false_block)); //2
    current_block->insert_goto(true_block);
    current_block->insert_goto(false_block);
}

void BranchInstruction::substitute(BasicBlockPtr old_block, BasicBlockPtr new_block) {
    auto num = use_list.at(1)->getValue() == old_block ? 1 : 2;
    old_block->delete_user(this);
    this->use_list.at(num) = new Use(this, new_block);
}

JumpInstruction::JumpInstruction(BasicBlockPtr jump_block, BasicBlockPtr current_block):
    Instruction(jump_block->get_value_return_type()->getContext()->getVoidType(),
        ValueType::JumpInst, current_block) {
    this->add_use(new Use(this, jump_block));
    current_block->insert_goto(jump_block);
}

void JumpInstruction::substitute(BasicBlockPtr old_bb, BasicBlockPtr new_bb) {
    this->delete_use(use_list.at(0));
    this->add_use(new Use(this, new_bb));
}

AllocaInstruction::AllocaInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block):
        Instruction(return_type->getContext()->getPointerType(return_type),
            ValueType::AllocaInst, basic_block) {
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
    if (function->get_value_return_type() !=
        get_value_return_type()/*VOID*/) {
        auto value = use_list.at(0)->getValue();
        value->get_value_return_type()->print(out);
        out << " ";
        value->print(out);
    }
    else {
        out << "void";
    }
}

ReturnInstruction::ReturnInstruction(ValueReturnTypePtr return_type, FunctionPtr function, ValuePtr ret_val, BasicBlockPtr basic_block):
    Instruction(return_type, ValueType::ReturnInst, basic_block) {
    if (ret_val) {
        this->add_use(new Use(this, ret_val));
    }
    this->function = function;
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

PhiInstruction::PhiInstruction(AllocaInstructionPtr alloca_inst, BasicBlockPtr basic_block):
    Instruction(alloca_inst->get_object_type(), ValueType::PhiInst, basic_block){
    alloca = alloca_inst;
    //do not use the upper alloca
}

void PhiInstruction::print_full(std::ostream &out) {
    print(out);
    out << " = phi ";
    alloca->get_object_type()->print(out);
    out << " ";
    auto it = options.begin();
    while (it != options.end()) {
        out << "[";
        it->second->print(out);
        out << ", %";
        it->first->print(out);
        out << "]";
        if (std::next(it) != options.end()) {
            out << ", ";
        }
        it++;
    }
    //this->alloca->print(out);
}

void PhiInstruction::add_option(ValuePtr value, BasicBlockPtr basic_block) {
    options[basic_block] = value;
    use_list.push_back(new Use(this, value));
    value->add_user(this);
    //TODO !!! CHECK
}

//JUST FOR LOAD!!! OTHERS MAY ERR
void Instruction::substitute_instruction(ValuePtr value) {
    for (auto user:user_list) {
        user->replace_use(this, value);
        value->add_user(user);
    }
}

PCInstruction::PCInstruction(BasicBlockPtr basic_block):
    Instruction(basic_block->getContext()->getVoidType(), ValueType::PCInst, basic_block) {
    basic_block->mark_pc(this);
}

void PCInstruction::add_edge(ValuePtr from, PhiInstructionPtr to) {
    //maintain use/def
    auto inst_from = dynamic_cast<InstructionPtr>(from);
    if (inst_from) {
        use.insert(inst_from);
    }
    auto arg = dynamic_cast<ArgumentPtr>(from);
    if (arg) {
        use.insert(arg);
    }
    if (use.find(to) == use.end()) {
        def.insert(to);
    }

    //maintain graph
    PCNode * from_node = nullptr, *to_node = nullptr;
    for (auto node: nodes) {
        if (*node == from) {
            from_node = node;
        } else if (*node == to) {
            to_node = node;
        }
    }
    if (!from_node) {
        from_node = new PCNode(from);
        nodes.push_back(from_node);
    }
    if (!to_node) {
        to_node = new PCNode(to);
        nodes.push_back(to_node);
    }
    from_node->insert_child(to_node);
    to_node->insert_ancestor(from_node);
    //TODO CHECK!!!
    //times cal in PHI 用于临时寄存器
    //use_list.push_back(new Use(to, from));
}

PCNode::PCNode(ValuePtr value): value(value) {}
bool PCNode::operator==(ValuePtr other) {
    return value == other;
}

void PCInstruction::print_full(std::ostream &out) {
    out << "pc: ";
    for (auto node: nodes) {
        for (auto edge: node->children) {
            node->value->print(out);
            out << "=>";
            edge->value->print(out);
            out << " ";
        }
    }
}
