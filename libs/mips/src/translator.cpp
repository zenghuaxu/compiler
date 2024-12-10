//
// Created by LENOVO on 2024/11/19.
//

#include "../../llvm/include/ir/globalValue.h"
#include "../include/mips.h"

#include "../include/mipsReg.h"
#include "../include/translator.h"

#include "../../llvm/include/llvm.h"
#include "../../llvm/include/ir/instructions.h"
#include "../../llvm/include/llvmContext.h"
#include "../../frontend/include/visitor.h"
#include "../../llvm/include/ir/constant.h"
#include "../include/data.h"
#include "../include/mipsInst.h"
#include "../../llvm/include/ir/function.h"
#include "../../llvm/include/ir/tmp_value.h"

DataPtr Translator::translate(GlobalValuePtr value) {
    auto type = value->global_value_type->get_ele_type();
    DataPtr data;
    if (type == type->getContext()->getCharType()) {
        data = new ByteData(value->name);
    }
    else {
        data = new WordData(value->name);
    }

    if (value->init_string.empty() && value->init_vector.empty()) {
        data->set_zero_num(value->global_value_type->get_length());
        return data;
    }

    if (typeid(*value->global_value_type) == typeid(ValueArrayType)) {
        auto array_type = dynamic_cast<ValueArrayTypePtr>(value->global_value_type);
        if (!value->init_vector.empty()) {
            int i;
            for (i = 0; i < value->init_vector.size(); i++) {
                data->put_value(value->init_vector[i]);
            }
            for (; i < array_type->get_length(); i++) {
                data->put_value(0);
            }
        }
        else {
            assert(array_type->get_ele_type() == array_type->getContext()->getCharType());
            data = new StringData(value->name, value->init_string,
                array_type->get_length());
        }
    }
    else {
        data->put_value(value->init_vector.at(0));
    }
    return data;
}

void Translator::translate(FunctionPtr function, std::vector<MipsInstPtr> &insts) {
    auto name = function->get_name();
    auto offset = new DynamicOffset(0);
    new Tag(insts, name + "_begin");
    //函数开始：申请栈空间，add sp什么的
    new ICode(manager->sp, manager->sp, new MemOffset(false, offset, 0, 1), ICodeOp::subiu, insts);

    //函数开始：去拿到参数，即去取映射参数对应的mem或reg
    int i = 0;
    for (auto arg: function->args) {
        if (i < A_REG_NUM) {
            manager->value_reg_map[arg] = manager->areg.at(i);
        }
        else {
            manager->value_reg_map[arg] = new MemOffset(false, offset, -i * 4, 4);
        }
        i++;
    }

    //alloc global
    alloc_global(function, offset);

    i = 0;
    for (auto block:function->blocks) {
        translate(block, insts, offset, ++i, name);
    }
}

void Translator::translate(BasicBlockPtr bb, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset,
    int no, std::string &name) {
    new Tag(insts, name + "_" + std::to_string(bb->id));
    for (auto it : bb->use_list) {
        auto inst = dynamic_cast<InstructionPtr>(it->getValue());
        if (typeid(*inst) == typeid(AllocaInstruction)) {
            translate(dynamic_cast<AllocaInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(UnaryOpInstruction)) {
            translate(dynamic_cast<UnaryOpInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(BinaryInstruction)) {
            translate(dynamic_cast<BinaryInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(CompareInstruction)) {
            translate(dynamic_cast<CompareInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(BranchInstruction)) {
            translate(dynamic_cast<BranchInstructionPtr>(inst), insts);
        } else if (typeid(*inst) == typeid(JumpInstruction)) {
            translate(dynamic_cast<JumpInstructionPtr>(inst), insts);
        } else if (typeid(*inst) == typeid(ZextInstruction)) {
            translate(dynamic_cast<ZextInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(TruncInstruction)) {
            translate(dynamic_cast<TruncInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(CallInstruction)) {
            translate(dynamic_cast<CallInstructionPtr>(inst), insts, bb->get_function(), offset);
        } else if (typeid(*inst) == typeid(ReturnInstruction)) {
            translate(bb->function->name == "main", dynamic_cast<ReturnInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(LoadInstruction)) {
            translate(dynamic_cast<LoadInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(StoreInstruction)) {
            translate(dynamic_cast<StoreInstructionPtr>(inst), insts);
        } else if (typeid(*inst) == typeid(GetElementPtrInstruction)) {
            translate(dynamic_cast<GetElementPtrInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(InputInstruction)) {
            translate(dynamic_cast<InputInstructionPtr>(inst), insts, offset);
        } else if (typeid(*inst) == typeid(OutputInstruction)) {
            translate(dynamic_cast<OutputInstructionPtr>(inst), insts);
        } else if (typeid(*inst) == typeid(PCInstruction)) {
            translate(dynamic_cast<PCInstructionPtr>(inst), insts, offset);
        } else {
            throw std::exception();
        }
    }
}

RegPtr Translator::mem_to_reg(ValuePtr value, std::vector<MipsInstPtr> &insts,
    bool left, bool const_to_reg) {
    //for global variable, la to reg
    if (typeid(*value) == typeid(GlobalValue)) {
        auto global_val = dynamic_cast<GlobalValuePtr>(value);
        auto reg = left ? get_l_swap() : get_r_swap();
        new LaCode(reg, "." + global_val->name, insts);
        return reg;
    }
    if (manager->value_reg_map.find(value) != manager->value_reg_map.end()) {
        //execute mem to reg
        if (typeid(*manager->value_reg_map.at(value)) == typeid(MemOffset)) {
            auto offset = dynamic_cast<MemOffsetPtr>(manager->value_reg_map.at(value));
            auto reg = left ? get_l_swap() : get_r_swap();
            new MemCode(reg, manager->sp, offset,
                offset->get_align_size() == 1 ? MemCodeOp::lbu : MemCodeOp::lw, insts);
            return reg;
        }
        //have allocated a reg
        return manager->value_reg_map.at(value);
    }
    //not alloc yet, assert is a constant
    if (const_to_reg) {
        assert(typeid(*value) == typeid(Constant));
        auto imm = dynamic_cast<ConstantPtr>(value)->get_value();
        auto alloc_reg = left ? get_l_swap() : get_r_swap();
        new ICode(alloc_reg, alloc_reg, imm, ICodeOp::li, insts);
        return alloc_reg;
    }
    return nullptr;
    //constant and no need to alloc a reg
}

RegPtr Translator::alloc_rd(ValuePtr inst, DynamicOffsetPtr offset) {
    auto map = manager->value_reg_map;
    if (map.find(inst) == map.end()) {
        return alloc_tmp(inst, true, offset);
        //not alloc yet, should alloc tmp
    }
    if (typeid(*map.at(inst)) == typeid(MemOffset)) {
        return  get_l_swap();
        //in the memory, to the swap reg
    }
    return manager->value_reg_map.at(inst);
}

void Translator::reg_to_mem(ValuePtr value, std::vector<MipsInstPtr> &insts, RegPtr reg) {
    if (manager->value_reg_map.find(value) != manager->value_reg_map.end() &&
        typeid(*manager->value_reg_map.at(value)) == typeid(MemOffset)) {
        auto offset = dynamic_cast<MemOffsetPtr>(manager->value_reg_map.at(value));
        new MemCode(reg, manager->sp, offset,
            offset->get_align_size() == 1 ? MemCodeOp::sb : MemCodeOp::sw, insts);
    }
    release_reg(value, false);
}

void Translator::alloc_global(FunctionPtr function, DynamicOffsetPtr offset) {
    function->create_conflict_graph();
    function->global_register_map(manager->save, manager->value_reg_map, offset);
}

RegPtr Translator::alloc_tmp(ValuePtr value, bool left, DynamicOffsetPtr offset) {
    for (auto tmp: manager->tmp) {
        if (!tmp->check_occupied()) {
            tmp->mark_occupied();
            manager->value_reg_map[value] = tmp;
            return tmp;
        }
    }
    manager->value_reg_map[value] = new MemOffset(offset, 4, 4);
    return left ? get_l_swap() : get_r_swap();
}

void Translator::release_reg(ValuePtr value, bool add_use) {
    auto inst = dynamic_cast<InstructionPtr>(value);
    if (!inst) {return;}
    if (manager->value_reg_map.find(value) != manager->value_reg_map.end()) {
        auto reg = manager->value_reg_map.at(value);
        auto tmp = dynamic_cast<TmpRegPtr>(reg);
        if (tmp) {
            if (add_use) {
                if (typeid(*inst) == typeid(TmpValue)) {
                    tmp->release_occupied();
                }
                else if (inst->add_map_and_try_release()) {
                    tmp->release_occupied();
                }
            }
            else {
                if (typeid(*inst) == typeid(TmpValue)) {
                    return;
                }
                if (inst->try_release()) {
                    tmp->release_occupied();
                }
            }
        }
    }
}

SwapRegPtr Translator::get_l_swap() {
    return manager->swap.at(0);
}

SwapRegPtr Translator::get_r_swap() {
    return manager->swap.at(1);
}
