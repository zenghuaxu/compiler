//
// Created by LENOVO on 2024/12/19.
//

#ifndef LLVM_OPT_H
#define LLVM_OPT_H

struct tripleIR;

enum Op : int{
    NEG,
    NOT,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    EQ,
    NE,
    SGT,
    SGE,
    SLT,
    SLE,

    ZEXT,

    TRUC1,
    TRUC8,

    GETELEMENT,
};

enum inst_type {
    unary,
    binary,
    comp,
    zext,
    trunc1,
    trunc8,
    getelementptr,
};


struct tripleIR {
    int op_hash;
    ValuePtr lhs;
    ValuePtr rhs;
    bool operator==(const tripleIR& other) const {
        return op_hash == other.op_hash && lhs == other.lhs && rhs == other.rhs;
    }
};

inline std::unordered_map<UnaryOpType, Op> unary_op_map = {
    {UnaryOpType::NEG, Op::NEG},
    {UnaryOpType::NOT, Op::NOT},
};

inline std::unordered_map<BinaryOp, Op> binary_op_map = {
    {BinaryOp::ADD, Op::ADD},
    {BinaryOp::SUB, Op::SUB},
    {BinaryOp::MUL, Op::MUL},
    {BinaryOp::DIV, Op::DIV},
    {BinaryOp::MOD, Op::MOD},
};

inline std::unordered_map<CompOp, Op> comp_op_map = {
    {CompOp::EQ, Op::EQ},
    {CompOp::NE, Op::NE},
    {CompOp::SGT, Op::SGT},
    {CompOp::SGE, Op::SGE},
    {CompOp::SLT, Op::SLT},
    {CompOp::SLE, Op::SLT},
};

inline int get_value(int l, int r, int op) {
    switch (op) {
        case NEG:
            return -l;
        case static_cast<int>(Op::NOT):
            return !l;
        case static_cast<int>(Op::ADD):
            return l + r;
        case static_cast<int>(Op::SUB):
            return l - r;
        case static_cast<int>(Op::MUL):
            return l * r;
        case static_cast<int>(Op::DIV):
            return l / r;
        case static_cast<int>(Op::MOD):
            return l % r;
        case EQ:
            return l == r;
        case static_cast<int>(Op::NE):
            return l != r;
        case static_cast<int>(Op::SGT):
            return l > r;
        case static_cast<int>(Op::SGE):
            return l >= r;
        case static_cast<int>(Op::SLT):
            return l < r;
        case static_cast<int>(Op::SLE):
            return l <= r;
        case static_cast<int>(Op::ZEXT):
            return l;
        case static_cast<int>(Op::TRUC1):
            return l & 1;
        case static_cast<int>(Op::TRUC8):
            return l & 0xff;
        default:
            return 0;
    }
}

#endif //LLVM_OPT_H
