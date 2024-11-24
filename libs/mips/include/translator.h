//
// Created by LENOVO on 2024/11/19.
//

#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include "../../llvm/include/llvm.h"
#include "mips.h"
#include "mipsManager.h"

class Translator {
    public:
    explicit Translator(MipsManagerPtr manager): manager(manager) {}

    static DataPtr translate(GlobalValuePtr value);

    void translate(FunctionPtr function, std::vector<MipsInstPtr> &insts);

    void translate(BasicBlockPtr bb, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset, int no, std::string &name);

    void translate(AllocaInstructionPtr alloca_instruction, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(UnaryOpInstructionPtr unary, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(BinaryInstructionPtr bi, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(CompareInstructionPtr comp, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(BranchInstructionPtr branch, std::vector<MipsInstPtr> &insts);

    void translate(JumpInstructionPtr jump, std::vector<MipsInstPtr> &insts);

    void translate(TruncInstructionPtr trunc, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(ZextInstructionPtr zext, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(LoadInstructionPtr load, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(StoreInstructionPtr load, std::vector<MipsInstPtr> &insts);

    void translate(InputInstructionPtr in, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(OutputInstructionPtr out, std::vector<MipsInstPtr> &insts);

    void translate(CallInstructionPtr call, std::vector<MipsInstPtr> &insts, FunctionPtr cur_func, DynamicOffsetPtr pre_offset);

    void translate(bool in_main, ReturnInstructionPtr ret, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    void translate(GetElementPtrInstructionPtr getelement, std::vector<MipsInstPtr> &insts, DynamicOffsetPtr offset);

    RegPtr mem_to_reg(ValuePtr value, std::vector<MipsInstPtr> &insts, bool left, bool const_to_reg);

    RegPtr alloc_rd(InstructionPtr inst, DynamicOffsetPtr offset);

    void reg_to_mem(ValuePtr value, std::vector<MipsInstPtr> &insts, RegPtr reg);

    void alloc_global(FunctionPtr function, DynamicOffsetPtr offset);

    RegPtr alloc_tmp(ValuePtr value, bool left, DynamicOffsetPtr offset);

    void release_reg(ValuePtr value, bool add_use);

    SwapRegPtr get_l_swap();

    SwapRegPtr get_r_swap();
private:
    MipsManagerPtr manager;
};

#endif //TRANSLATOR_H
