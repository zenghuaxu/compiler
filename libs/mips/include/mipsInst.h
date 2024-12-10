//
// Created by LENOVO on 2024/11/21.
//

#ifndef MIPSINST_H
#define MIPSINST_H
#include <string>
#include <utility>
#include "mips.h"
#include "../../llvm/include/ir/instructions.h"

class MipsInst {
    public:
    virtual ~MipsInst() = default;

    MipsInst(std::vector<MipsInstPtr> &insts) {
        insts.push_back(this);
    }

    virtual void print(std::ostream &out) = 0;
};

class Tag: public MipsInst {
    public:
    Tag(std::vector<MipsInstPtr> &insts, std::string tag): MipsInst(insts), name(std::move(tag)) {}
    void print(std::ostream &out) override;
    private:
    std::string name;
};

enum class RCodeOp {
        addu,
        subu,
        slt,
        sle,
        sgt,
        sge,
        seq,
        sne,
        mul,
        div,

        move,

        mod,
};

inline std::unordered_map<BinaryOp, RCodeOp> binaryOpMap = {
        {BinaryOp::ADD, RCodeOp::addu},
        {BinaryOp::SUB, RCodeOp::subu},
        {BinaryOp::MUL, RCodeOp::mul},
        {BinaryOp::DIV, RCodeOp::div},
        {BinaryOp::MOD, RCodeOp::mod},
};

inline std::unordered_map<CompOp, RCodeOp> compareOpMap = {
        {CompOp::EQ, RCodeOp::seq},
        {CompOp::NE, RCodeOp::sne},
        {CompOp::SGE, RCodeOp::sge},
        {CompOp::SGT, RCodeOp::sgt},
        {CompOp::SLE, RCodeOp::sle},
        {CompOp::SLT, RCodeOp::slt},
};

class RCode: public MipsInst {
    public:
    RCode(RegPtr rs, RegPtr rt, RegPtr rd, BinaryOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), rd(rd), op(binaryOpMap.at(op)) {}
    RCode(RegPtr rs, RegPtr rt, RegPtr rd, CompOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), rd(rd), op(compareOpMap.at(op)) {}
    RCode(RegPtr rs, RegPtr rt, RegPtr rd, RCodeOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), rd(rd), op(op) {}

    void print(std::ostream &out) override;
    private:
    RCodeOp op;
    RegPtr rs, rt, rd;
};

enum class ICodeOp {
    addiu,
    subiu,
    mul,
    div,
    andi,

    li,
};

inline std::unordered_map<BinaryOp, ICodeOp> binaryOpIMap = {
    {BinaryOp::ADD, ICodeOp::addiu},
    {BinaryOp::SUB, ICodeOp::subiu},
    {BinaryOp::MUL, ICodeOp::mul},
    {BinaryOp::DIV, ICodeOp::div},
};

class ICode: public MipsInst {
public:
    ICode(RegPtr rs, RegPtr rt, int imm, ICodeOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), immediate(imm), offset(nullptr), op(op) {}
    ICode(RegPtr rs, RegPtr rt, int imm, BinaryOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), immediate(imm), offset(nullptr), op(binaryOpIMap.at(op)) {}
    ICode(RegPtr rs, RegPtr rt, MemOffsetPtr offset, ICodeOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rs(rs), rt(rt), immediate(0), offset(offset), op(op) {}

    void print(std::ostream &out) override;

    private:
    RegPtr rs;
    RegPtr rt;//destination
    ICodeOp op;
    int immediate;
    MemOffsetPtr offset;
};

enum class MemCodeOp {
    lw,
    sw,
    lbu,
    sb
};

class MemCode: public MipsInst {
    public:
    MemCode(RegPtr rt, RegPtr base, MemOffsetPtr offset, MemCodeOp op, std::vector<MipsInstPtr> &insts):
        MipsInst(insts),
        rt(rt), base(base), offset(offset), op(op) {}

    void print(std::ostream &out) override;

    private:
    RegPtr rt, base;
    MemOffsetPtr offset;
    MemCodeOp op;
};

enum class BranchCodeOp {
    beq,
    bne,
};

class BranchCode: public MipsInst {
    public:
    BranchCode(RegPtr rs, RegPtr rt, BranchCodeOp op, std::string name, std::vector<MipsInstPtr> &insts):
        MipsInst(insts), rs(rs), rt(rt), op(op), label_name(std::move(name)) {}

    void print(std::ostream &out) override;

    private:
    RegPtr rs, rt;
    BranchCodeOp op;
    std::string label_name;
};

class JumpCode: public MipsInst {
    public:
    JumpCode(std::string name, std::vector<MipsInstPtr> &insts):
        MipsInst(insts), label_name(std::move(name)) {}

    void print(std::ostream &out) override;

    private:
    std::string label_name;
};

class JalCode: public MipsInst {
public:
    JalCode(std::vector<MipsInstPtr> &insts, std::string name):
        MipsInst(insts), name(std::move(name)) {}

    void print(std::ostream &out) override;
private:
    std::string name;
};

class JrCode: public MipsInst {
public:
    JrCode(RegPtr ra, std::vector<MipsInstPtr> &insts):
        MipsInst(insts), ra(ra) {}

    void print(std::ostream &out) override;
private:
    RegPtr ra;
};

class SysCallCode: public MipsInst {
    public:
    explicit SysCallCode(std::vector<MipsInstPtr> &insts):
        MipsInst(insts) {}
void print(std::ostream &out) override;
private:
};

class LaCode: public MipsInst {
public:
    LaCode(RegPtr rd, std::string label_name, std::vector<MipsInstPtr> &insts):
        MipsInst(insts), rd(rd), label_name(std::move(label_name)) {}

    void print(std::ostream &out) override;
    private:
    std::string label_name;
    RegPtr rd;
};

#endif //MIPSINST_H
