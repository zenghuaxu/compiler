//
// Created by LENOVO on 2024/11/24.
//
#include "../../llvm/include/ir/globalValue.h"
#include "../include/mips.h"

#include "../include/mipsReg.h"
#include "../include/translator.h"

#include "../../frontend/include/visitor.h"
#include "../../llvm/include/llvmContext.h"
#include "../../llvm/include/ir/constant.h"
#include "../../llvm/include/ir/function.h"
#include "../../llvm/include/ir/tmp_value.h"
#include "../include/mipsInst.h"

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
    if (!rt) {
        auto imm = dynamic_cast<ConstantPtr>(use)->get_value();
        if (unary->op == UnaryOpType::NEG) {
            new ICode(nullptr, rd, -imm, ICodeOp::li, insts);
        }
        else {
            new ICode(nullptr, rd, imm == 0, ICodeOp::li, insts);
        }
    }
    else {
        if (unary->op == UnaryOpType::NEG) {
            new RCode(manager->zero, rt, rd, BinaryOp::SUB, insts);
        }
        else {
            new RCode(manager->zero, rt, rd, CompOp::EQ, insts);
        }
    }
    //tailing
    release_reg(use, true);
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
    if (!rs && !rt) {
        auto imm_l = dynamic_cast<ConstantPtr>(left)->get_value();
        auto imm_r = dynamic_cast<ConstantPtr>(right)->get_value();
        auto imm = cal(imm_l, imm_r, bi->op);
        new ICode(nullptr, rd, imm, ICodeOp::li, insts);
    }
    else if (!rs /*IMM*/) {
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
    release_reg(left, true);
    release_reg(right, true);
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
    release_reg(left, true);
    release_reg(right, true);
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

    release_reg(use, true);
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

    auto rs = mem_to_reg(use, insts, true, true);
    RegPtr rd = alloc_rd(trunc, offset);

    auto type = trunc->get_value_return_type();
    auto context = type->getContext();

    auto bitType = context->getBitType();

    if (type == bitType) {
        new ICode(rs, rd, 1, ICodeOp::andi, insts);
    }
    else {
        new ICode(rs, rd, 0xff, ICodeOp::andi, insts);
    }

    release_reg(use, true);
    reg_to_mem(trunc, insts, rd);
}

void Translator::translate(ZextInstructionPtr zext, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    if (manager->value_reg_map.find(zext->use_list.at(0)->getValue()) ==
                manager->value_reg_map.end()) {
        RegPtr rd = alloc_rd(zext, offset);
        new ICode(rd, rd, dynamic_cast<ConstantPtr>(zext->use_list.at(0)->getValue())->get_value()
            , ICodeOp::li, insts);
        release_reg(zext, false);
    }
    else {
        manager->value_reg_map[zext] = manager->value_reg_map.at(zext->use_list.at(0)->getValue());
    }
    //TODO CHECK MEM ACCESS!!
}

void Translator::translate(LoadInstructionPtr load, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    auto use = load->use_list.at(0)->getValue();

    auto rs = mem_to_reg(use, insts, true, false);
    RegPtr rd = alloc_rd(load, offset);

    auto type = load->get_value_return_type();
    auto context = type->getContext();

    auto CharType = context->getCharType();
    auto BitType = context->getBitType();

    if (type != CharType && type != BitType) {
        new MemCode(rd, rs, nullptr, MemCodeOp::lw, insts);//TODO OFF SET NULL
    }
    else {
        new MemCode(rd, rs, nullptr, MemCodeOp::lbu, insts);//TODO OFF SET NULL
    }//TODO

    release_reg(use, true);
    reg_to_mem(load, insts, rd);
}

void Translator::translate(StoreInstructionPtr store, std::vector<MipsInstPtr> &insts) {
    auto value = store->use_list.at(0)->getValue();
    auto addr = store->use_list.at(1)->getValue();

    auto rt = mem_to_reg(value, insts, true, true);
    auto base = mem_to_reg(addr, insts, false, false);

    auto type = value->get_value_return_type();
    auto context = type->getContext();

    auto CharType = context->getCharType();
    auto BitType = context->getBitType();

    if (type != CharType && type != BitType) {
        new MemCode(rt, base, nullptr, MemCodeOp::sw, insts);//TODO OFF SET NULL
    }
    else {
        new MemCode(rt, base, nullptr, MemCodeOp::sb, insts);//TODO OFF SET NULL
    }

    release_reg(value, true);
    release_reg(addr, true);
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

    release_reg(use, true);
}

void Translator::translate(GetElementPtrInstructionPtr getelement, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr dy_offset) {
    auto base = getelement->use_list.at(0)->getValue();
    auto offset = getelement->use_list.at(1)->getValue();

    auto rs = mem_to_reg(base, insts, true, false);
    auto rt = mem_to_reg(offset, insts, false, false);

    RegPtr rd = alloc_rd(getelement, dy_offset);
    assert(rs);
    //i32
    if (dynamic_cast<PointerTypePtr>(getelement->right_val)
        ->get_referenced_type()->get_ele_type() ==
        getelement->right_val->getContext()->getIntType()) {
        if (!rt) {
            auto imm = dynamic_cast<ConstantPtr>(offset)->get_value();
            new ICode(rs, rd, imm * 4, ICodeOp::addiu, insts);
        }
        else {
            //THIS PLACE ERROR, WHY???
            //ALWAYS PUT ON SWAP2
            new ICode(rt, manager->swap.at(1), 4, ICodeOp::mul, insts);
            new RCode(rs, manager->swap.at(1), rd, RCodeOp::addu, insts);
        }
    }
    //i8
    else {
        if (!rt) {
            auto imm = dynamic_cast<ConstantPtr>(offset)->get_value();
            new ICode(rs, rd, imm, ICodeOp::addiu, insts);
        }
        else {
            new RCode(rs, rt, rd, RCodeOp::addu, insts);
        }
    }

    release_reg(base, true);
    release_reg(offset, true);
    reg_to_mem(getelement, insts, rd);
}

void Translator::translate(CallInstructionPtr call, std::vector<MipsInstPtr> &insts, FunctionPtr cur_func,
    DynamicOffsetPtr pre_offset) {
    //STACK OPERATIONS
    new MemOffset(pre_offset, 0, 4);//上个栈空对齐
    int func_arg = call->function->args.size();
    auto param_size = func_arg * 4;
    auto s_reg_size = cur_func->saved_reg_used_num * 4;
    auto stack_down = param_size + s_reg_size + 4;
    auto offset =  new DynamicOffset(0);

    //首先，存自己的RA和SAVED REG
    //然后，传递参数，所有参数预留空间，且每个四字节对齐。前四个存到a0 - a4， 后面几个存在栈上，让后面的func可以算出来
    //ARGUMENT
    for (int i = 0; i < A_REG_NUM && i < func_arg; i++) {
        auto rs = mem_to_reg(call->use_list.at(i)->getValue(), insts, true, true);
        new RCode(rs, rs, manager->areg.at(i), RCodeOp::move, insts);
    }
    //ARGUMENT
    for (int i = func_arg - 1; i >= A_REG_NUM; i--) {
        auto rs = mem_to_reg(call->use_list.at(i)->getValue(), insts, true, true);
        new MemCode(rs, manager->sp, new MemOffset(true, offset, stack_down - i * 4
            , 4), MemCodeOp::sw,insts); //TODO CHECK SW is true?
    }

    new ICode(manager->sp, manager->sp, -stack_down, ICodeOp::addiu, insts);
    //RA
    new MemCode(manager->ra, manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::sw,insts);
    //S_REG
    for (int i = 0; i < cur_func->saved_reg_used_num; i++) {
        new MemCode(manager->save.at(i), manager->sp, new MemOffset(offset, 4, 4), MemCodeOp::sw,insts);
    }
    new MemOffset(offset, func_arg * 4, 4); //just alloc, assignment has been done at first

    //跳转即可
    new JalCode(insts, call->function->name + "_begin");

    auto position = 4;
    //恢复现场，恢复RA和SAVED REG
    //RA
    new MemCode(manager->ra, manager->sp, new MemOffset(false, offset, position, 4), MemCodeOp::lw, insts);
    position += 4;
    //SAVED_REG
    for (int i = 0; i < cur_func->saved_reg_used_num; i++) {
        new MemCode(manager->save.at(i), manager->sp, new MemOffset(false, offset, position, 4), MemCodeOp::lw,insts);
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
        release_reg(it->getValue(), true);
    }
}

void Translator::translate(bool in_main, ReturnInstructionPtr ret, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset) {
    //return：：把返回值存到v0
    if (!ret->use_list.empty()) {
        auto rs = mem_to_reg(ret->use_list.at(0)->getValue(), insts, true, true);
        new RCode(rs, rs, manager->vreg.at(0), RCodeOp::move, insts);
    }
    //return：：释放栈空间
    new ICode(manager->sp, manager->sp, new MemOffset(false, offset, 0, 1), ICodeOp::addiu, insts);
    //跳转jar
    if (in_main) {
        auto v0 = manager->vreg.at(0);
        new ICode(v0, v0, 10, ICodeOp::li, insts);
        new SysCallCode(insts);
    } else {
        new JrCode(manager->ra, insts);
    }
}

void Translator::translate(PCInstructionPtr pc, std::vector<MipsInstPtr> &insts,
    DynamicOffsetPtr offset) {
    std::set<PCNodePtr> f_nodes;
    while(f_nodes.size() < pc->nodes.size()) {
        auto flag = 1;
        for (auto node: pc->nodes) {
            if (f_nodes.find(node) != f_nodes.end()) { continue; }
            if (node->ancestor.empty()) {
                f_nodes.insert(node);
                continue;
            }
            if (node->children.empty()) {
                auto phi = dynamic_cast<PhiInstructionPtr>(node->value);
                assert(phi != nullptr);
                auto rd = alloc_rd(phi, offset);
                //assert: just 1
                for (auto pre:node->ancestor) {
                    auto from = pre->value;
                    //TODO CHECK REALY ALLOCED??
                    auto rs = mem_to_reg(from, insts, true, false);
                    if (rs) {
                        new RCode(rs, rs, rd, RCodeOp::move, insts);
                    }
                    else {
                        auto imm = dynamic_cast<ConstantPtr>(from)->get_value();
                        new ICode(nullptr, rd, imm, ICodeOp::li, insts);
                    }
                    release_reg(from, true);
                    reg_to_mem(phi, insts, rd);
                    pre->delete_ancestor(node);
                }
                f_nodes.insert(node);
                flag = 0;
                break;
            }
        }
        if (flag) {
            for (auto node: pc->nodes) {
                if (f_nodes.find(node) != f_nodes.end()) { continue; }
                //delete origin
                auto it = node->children.begin();
                node->children.erase(it);
                auto to = dynamic_cast<PhiInstructionPtr>((*it)->value);
                (*it)->delete_ancestor(node);

                //add new
                auto tmp = new TmpValue();
                pc->add_edge(tmp, to);

                //add move, a-> a'
                auto rd = alloc_rd(tmp, offset);
                auto rs = mem_to_reg(node->value, insts, true, false);
                new RCode(rs, rs, rd, RCodeOp::move, insts);
                reg_to_mem(tmp, insts, rd);
                break;
            }
        }
    }
}

int Translator::cal(int left, int right, BinaryOp op) {
    switch (op) {
        case BinaryOp::ADD:
            return left + right;
        case BinaryOp::SUB:
            return left - right;
        case BinaryOp::MUL:
            return left * right;
        case BinaryOp::DIV:
            return left / right;
        case BinaryOp::MOD:
            return left % right;
        default:
            throw std::invalid_argument("Unknown BinaryOp");
    }
}
