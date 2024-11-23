//
// Created by LENOVO on 2024/11/19.
//

#include "../../llvm/include/ir/globalValue.h"
#include "../include/mips.h"

#include "../include/mipsReg.h"
#include "../include/translator.h"

#include "../../frontend/include/visitor.h"
#include "../../llvm/include/llvmContext.h"
#include "../../llvm/include/ir/constant.h"
#include "../include/data.h"
#include "../include/mipsInst.h"

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
        type = dynamic_cast<ValueArrayTypePtr>(value->global_value_type);
        if (!value->init_vector.empty()) {
            int i;
            for (i = 0; i < value->init_vector.size(); i++) {
                data->put_value(value->init_vector[i]);
            }
            for (; i < type->get_length(); i++) {
                data->put_value(0);
            }
        }
        else {
            data = new StringData(value->name, value->init_string);
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
    new ICode(manager->sp, manager->sp, new MemOffset(true, offset, 0, 1), ICodeOp::subiu, insts);

    //函数开始：去拿到参数，即去取映射参数对应的mem或reg
    int i = 0;
    for (auto arg: function->args) {
        if (i < A_REG_NUM) {
            manager->value_reg_map[arg] = manager->areg.at(i);
        }
        else {
            manager->value_reg_map[arg] = new MemOffset(true, offset, -i * 4, 4);
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
            //DO NOTHING; //TODO MEM ACC WRONG!!
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
        } else {
            throw std::exception();
        }
    }
}

void Translator::translate(AllocaInstructionPtr alloca_instruction, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    MemOffsetPtr mem;
    if (alloca_instruction->get_object_type()->get_ele_type() ==
        alloca_instruction->get_object_type()->getContext()->getCharType()) {
        mem = new MemOffset(offset, alloca_instruction->get_object_type()->get_byte_size(),
            1);
    }
    else {
        mem = new MemOffset(offset, alloca_instruction->get_object_type()->get_byte_size(),
            4);
    }
    //manager->value_reg_map[alloca_instruction] = mem;

    auto rd = alloc_rd(alloca_instruction, offset);
    new ICode(manager->sp, rd, mem, ICodeOp::addiu, insts);
    reg_to_mem(alloca_instruction, insts, rd);
}

void Translator::translate(UnaryOpInstructionPtr unary, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto use = unary->use_list.at(0)->getValue();

    //rs, rt, rd are all regs
    auto rt = mem_to_reg(use, insts, true, false);
    RegPtr rd = alloc_rd(unary, offset);

    //core inst
    assert(rt);
    if (unary->op == UnaryOpType::NEG) {
        new RCode(manager->zero, rt, rd, BinaryOp::SUB, insts);
    }
    else {
        new RCode(manager->zero, rt, rd, CompOp::EQ, insts);
    }

    //tailing
    release_reg(use);
    reg_to_mem(unary, insts, rd);
}

void Translator::translate(BinaryInstructionPtr bi, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto left = bi->get_lhs();
    auto right = bi->get_rhs();

    //rs, rt, rd are all regs
    auto rs = mem_to_reg(left, insts, true, false);
    auto rt =  mem_to_reg(right, insts, false, false);
    RegPtr rd = alloc_rd(bi, offset);

    //core inst
    if (!rs /*IMM*/) {
        auto imm = dynamic_cast<ConstantPtr>(left)->get_value();
        new ICode(nullptr, rd, imm, ICodeOp::li, insts);
        new RCode(rd, rt, rd, bi->getBinaryOp(), insts);
    }
    else if (!rt) {
        auto imm = dynamic_cast<ConstantPtr>(right)->get_value();
        if (bi->op != BinaryOp::MOD) {
            new ICode(rs, rd, imm, bi->getBinaryOp(), insts);
        }
        else {
            new ICode(nullptr, rd, imm, ICodeOp::li, insts);
            new RCode(rs, rd, rd, bi->getBinaryOp(), insts);
        }
    }
    else {
        assert(rs && rt);
        new RCode(rs, rt, rd, bi->getBinaryOp(), insts);
    }

    //tailing
    release_reg(left);
    release_reg(right);
    reg_to_mem(bi, insts, rd);
}

void Translator::translate(CompareInstructionPtr comp, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto left = comp->use_list.at(0)->getValue();
    auto right = comp->use_list.at(1)->getValue();

    //rs, rt, rd are all regs
    auto rs = mem_to_reg(left, insts, true, true);
    auto rt =  mem_to_reg(right, insts, false, true);
    RegPtr rd = alloc_rd(comp, offset);

    assert(rs && rt);
    new RCode(rs, rt, rd, comp->get_comp_op(), insts);

    //tailing
    release_reg(left);
    release_reg(right);
    reg_to_mem(comp, insts, rd);
}

void Translator::translate(BranchInstructionPtr branch, std::vector<MipsInstPtr> &insts) {
    auto use = branch->use_list.at(0)->getValue();

    auto true_block = dynamic_cast<BasicBlockPtr>(branch->use_list.at(1)->getValue());
    auto false_block = dynamic_cast<BasicBlockPtr>(branch->use_list.at(2)->getValue());
    auto func_name = true_block->get_function()->get_name();

    auto true_name = func_name + "_" +  std::to_string(true_block->id);
    auto false_name = func_name + "_" + std::to_string(false_block->id);

    auto rt = mem_to_reg(use, insts, true, false);
    if (!rt) {
        auto imm = dynamic_cast<ConstantPtr>(use)->get_value();
        if (imm) {
            new JumpCode(true_name, insts);
        }
        else {
            new JumpCode(false_name, insts);
        }
    }
    else {
        new BranchCode(manager->zero, rt, BranchCodeOp::bne, true_name,insts);
        new JumpCode(false_name, insts);
    }

    release_reg(use);
}

void Translator::translate(JumpInstructionPtr jump, std::vector<MipsInstPtr> &insts) {
    auto block = dynamic_cast<BasicBlockPtr>(jump->use_list.at(0)->getValue());
    auto func_name = block->get_function()->get_name();

    auto name = func_name + "_" +  std::to_string(block->id);
    new JumpCode(name, insts);
}

void Translator::translate(TruncInstructionPtr trunc, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto use = trunc->use_list.at(0)->getValue();

    auto rs = mem_to_reg(use, insts, true, false);
    RegPtr rd = alloc_rd(trunc, offset);

    auto type = trunc->get_value_return_type();
    auto context = type->getContext();

    auto bitType = context->getBitType();

    if (type == bitType) {
        new ICode(rs, rd, 1, ICodeOp::andi, insts);
    }
    else {
        new ICode(rs, rd, 0x11, ICodeOp::andi, insts);
    }

    release_reg(use);
    reg_to_mem(trunc, insts, rd);
}

void Translator::translate(LoadInstructionPtr load, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto use = load->use_list.at(0)->getValue();

    auto rs = mem_to_reg(use, insts, true, false);
    RegPtr rd = alloc_rd(load, offset);

    auto type = load->get_value_return_type();
    auto context = type->getContext();

    auto IntType = context->getIntType();

    if (type != IntType) {
        new MemCode(rd, rs, nullptr, MemCodeOp::lbu, insts);//TODO OFF SET NULL
    }
    else {
        new MemCode(rd, rs, nullptr, MemCodeOp::lw, insts);//TODO OFF SET NULL
    }

    release_reg(use);
    reg_to_mem(load, insts, rd);
}

void Translator::translate(StoreInstructionPtr store, std::vector<MipsInstPtr> &insts) {
    auto value = store->use_list.at(0)->getValue();
    auto addr = store->use_list.at(1)->getValue();

    auto rt = mem_to_reg(value, insts, true, true);
    auto base = mem_to_reg(addr, insts, false, false);

    auto type = value->get_value_return_type();
    auto context = type->getContext();

    auto IntType = context->getIntType();

    if (type != IntType) {
        new MemCode(rt, base, nullptr, MemCodeOp::sb, insts);//TODO OFF SET NULL
    }
    else {
        new MemCode(rt, base, nullptr, MemCodeOp::sw, insts);//TODO OFF SET NULL
    }

    release_reg(value);
    release_reg(addr);
}

void Translator::translate(InputInstructionPtr in, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto v0 = manager->vreg.at(0);
    if (in->ch_type) {
        new ICode(v0, v0, 12, ICodeOp::li, insts);
    }
    else {
        new ICode(v0, v0, 5, ICodeOp::li, insts);
    }
    new SysCallCode(insts);

    RegPtr rd = alloc_rd(in, offset);
    new RCode(v0, v0, rd, RCodeOp::move, insts);

    reg_to_mem(in, insts, rd);
}

void Translator::translate(OutputInstructionPtr out, std::vector<MipsInstPtr> &insts) {
    auto v0 = manager->vreg.at(0);
    auto a0 = manager->areg.at(0);

    auto use = out->use_list.at(0)->getValue();

    auto rs = mem_to_reg(use, insts, true, false);
    if (!rs) {
        auto imm = dynamic_cast<ConstantPtr>(use)->get_value();
        new ICode(a0, a0, imm, ICodeOp::li, insts);
    }
    else {
        new RCode(rs, rs, a0, RCodeOp::move, insts);
    }

    if (typeid(*out->value_return_type) == typeid(PointerType)) {
        new ICode(v0, v0, 4, ICodeOp::li, insts);
        new SysCallCode(insts);
    }
    else if (out->ch) {
        new ICode(v0, v0, 11, ICodeOp::li, insts);
        new SysCallCode(insts);
    }
    else {
        new ICode(v0, v0, 1, ICodeOp::li, insts);
        new SysCallCode(insts);
    }

    release_reg(use);
}

void Translator::translate(GetElementPtrInstructionPtr getelement, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr dy_offset) {
    auto base = getelement->use_list.at(0)->getValue();
    auto offset = getelement->use_list.at(1)->getValue();

    auto rs = mem_to_reg(base, insts, true, false);
    auto rt = mem_to_reg(offset, insts, false, false);

    RegPtr rd = alloc_rd(getelement, dy_offset);

    assert(rs);
    if (!rt) {
        auto imm = dynamic_cast<ConstantPtr>(offset)->get_value();
        new ICode(rs, rd, imm, ICodeOp::addiu, insts);
    }
    else {
        new RCode(rs, rt, rd, RCodeOp::addu, insts);
    }

    release_reg(base);
    release_reg(offset);
    reg_to_mem(getelement, insts, rd);
}

void Translator::translate(CallInstructionPtr call, std::vector<MipsInstPtr> &insts, FunctionPtr cur_func,
    DynamicOffsetPtr pre_offset) {
    //STACK OPERATIONS
    auto func_arg = call->function->args.size();
    auto param_size = func_arg * 4;
    auto s_reg_size = cur_func->saved_reg_used_num * 4;
    auto stack_down = param_size + s_reg_size + 4;
    auto offset =  new DynamicOffset(0);
    auto position = -16; //4 * 4

    //首先，存自己的RA和SAVED REG
    //然后，传递参数，所有参数预留空间，且每个四字节对齐。前四个存到a0 - a4， 后面几个存在栈上，让后面的func可以算出来
    //ARGUMENT
    for (int i = 0; i < A_REG_NUM && i < func_arg; i++) {
        auto rs = mem_to_reg(call->use_list.at(i)->getValue(), insts, true, true);
        new RCode(rs, rs, manager->areg.at(i), RCodeOp::move, insts);
    }
    new ICode(manager->sp, manager->sp, -stack_down, ICodeOp::addiu, insts);
    //ARGUMENT
    for (int i = 4; i < func_arg; i++) {
        auto rs = mem_to_reg(call->use_list.at(i)->getValue(), insts, true, true);
        new MemCode(rs, manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::sw,insts); //TODO CHECK SW is true?
        position -= 4;
    }

    //S_REG
    for (int i = 0; i < cur_func->saved_reg_used_num; i++) {
        new MemCode(manager->save.at(i), manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::sw,insts);
        position -= 4;
    }
    //RA
    new MemCode(manager->ra, manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::sw,insts);

    //跳转即可
    new JalCode(insts, call->function->name + "_begin");

    //恢复现场，恢复RA和SAVED REG
    //RA
    new MemCode(manager->ra, manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::lw, insts);
    position += 4;
    //SAVED_REG
    for (int i = cur_func->saved_reg_used_num - 1; i >= 0; i--) {
        new MemCode(manager->save.at(i), manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::lw,insts);
        position += 4;
    }
    //恢复栈空间
    new ICode(manager->sp, manager->sp, stack_down, ICodeOp::addiu, insts);//resume sp

    //取得返回值
    if (call->get_value_return_type() !=
        call->get_value_return_type()->getContext()->getVoidType()) {
        RegPtr rd = alloc_rd(call, pre_offset);
        //TODO CHECK IF OVERFLOW(MEM), IS IT RIGHT?
        auto v0 = manager->vreg.at(0);
        new RCode(v0, v0, rd, RCodeOp::move, insts);
        reg_to_mem(call, insts, rd);
    }

    //tail
    for (auto it: call->use_list) {
        release_reg(it->getValue());
    }
}

void Translator::translate(bool in_main, ReturnInstructionPtr ret, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset) {
    //return：：把返回值存到v0
    if (!ret->use_list.empty()) {
        auto rs = mem_to_reg(ret->use_list.at(0)->getValue(), insts, true, true);
        new RCode(rs, rs, manager->vreg.at(0), RCodeOp::move, insts);
    }
    //return：：释放栈空间
    new ICode(manager->sp, manager->sp, new MemOffset(true, offset, 0, 1), ICodeOp::addiu, insts);
    //跳转jar
    if (in_main) {
        auto v0 = manager->vreg.at(0);
        new ICode(v0, v0, 10, ICodeOp::li, insts);
        new SysCallCode(insts);
    } else {
        new JrCode(manager->ra, insts);
    }
}

RegPtr Translator::mem_to_reg(ValuePtr value, std::vector<MipsInstPtr> &insts,
    bool left, bool const_to_reg) {
    //for global variable, la to reg
    if (typeid(*value) == typeid(GlobalValue)) {
        auto global_val = dynamic_cast<GlobalValuePtr>(value);
        auto reg = left ? get_l_swap() : get_r_swap();
        new LaCode(reg, global_val->name, insts);
        return reg;
    }
    if (manager->value_reg_map.find(value) != manager->value_reg_map.end()) {
        //execute mem to reg
        if (typeid(*manager->value_reg_map.at(value)) == typeid(MemOffset)) {
            auto offset = dynamic_cast<MemOffsetPtr>(manager->value_reg_map.at(value));
            auto reg = left ? get_l_swap() : get_r_swap();
            new MemCode(reg, manager->sp ,offset,
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

RegPtr Translator::alloc_rd(InstructionPtr inst, DynamicOffsetPtr offset) {
    if (manager->value_reg_map.find(inst) == manager->value_reg_map.end()) {
        return alloc_tmp(inst, true, offset);
        //not alloc yet, should alloc tmp
    }
    if (typeid(*manager->value_reg_map.at(inst)) == typeid(MemOffsetPtr)) {
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

void Translator::release_reg(ValuePtr value) {
    auto inst = dynamic_cast<InstructionPtr>(value);
    if (!inst) {return;}
    if (manager->value_reg_map.find(value) != manager->value_reg_map.end()) {
        auto reg = manager->value_reg_map.at(value);
        auto tmp = dynamic_cast<TmpRegPtr>(reg);
        if (tmp && inst->add_map_and_try_release()) {//副作用：分配次数加一
            tmp->release_occupied();
        }
    }
}

SwapRegPtr Translator::get_l_swap() {
    return manager->swap.at(0);
}

SwapRegPtr Translator::get_r_swap() {
    return manager->swap.at(1);
}
