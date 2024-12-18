//
// Created by LENOVO on 2024/12/18.
//

#include <cstdint>
#include <cassert>

#include "../include/mips.h"
#include "../include/opt.h"
#include "../include/mipsReg.h"
#include "../include/mipsInst.h"
#include "../include/mipsManager.h"
#define N (32)

struct MulMagic {
    uint64_t m;
    unsigned int l;
};

MulMagic mulMagic(uint32_t d) {
    assert(d);
    unsigned int l = N - __builtin_clz(d - 1);
    //计算log 2 d
    uint64_t ground = (static_cast<uint64_t>(1) << (N + l)) / d;
    uint64_t upper = ((static_cast<uint64_t>(1) << (N + l)) + (static_cast<uint64_t>(1) << (l + 1))) / d;
    while ((upper >> 1) > (ground >> 1) && l > 0) {
        l--;
        upper >>= 1;
        ground >>= 1;
    }
    return (struct MulMagic){upper, l};
}

void Opt::generate_div(RegPtr rs, RegPtr rd, int imm, std::vector<MipsInstPtr> &insts,
    RegPtr swap1, RegPtr swap2, RegPtr zero)
{
    assert(imm);
    auto magic = mulMagic(abs(imm));
    auto m = magic.m;
    auto l = magic.l;
    if (m < static_cast<uint64_t>(1) << (N - 1)) {
        new ICode(rs, swap2, m, BinaryOp::MUL, insts);
        new Mfhi(swap2, insts);
    }
    else {
        auto mul = m - (static_cast<uint64_t>(1) << (N));
        new ICode(rs, swap2, mul, BinaryOp::MUL, insts);
        new Mfhi(swap2, insts);
        new RCode(rs, swap2, swap2, RCodeOp::addu, insts);
        //TODO OVERFLOW??
    }
    //assert to swap2


    if (l > 0) {
        new ICode(swap2, swap2, l, ICodeOp::sra, insts);
    }

    //notice:: cannot use rs again!!!
    new RCode(rs, zero, swap1, RCodeOp::slt, insts);
    new RCode(swap2, swap1, swap2, RCodeOp::addu, insts);

    //if imm minus, neg, else move swap2->rd
    if (imm < 0) {
        new RCode(zero, swap2, rd, RCodeOp::subu, insts);
    }
    else {
        new RCode(swap2, swap2, rd, RCodeOp::move, insts);
    }
}

void MipsManager::reduce_mul() {
    for (auto & inst : insts) {
        if (typeid(*inst) == typeid(ICode)) {
            auto icode = dynamic_cast<ICode *>(inst);
            icode->reduce_mul();
        }
    }
}

void ICode::reduce_mul() {
    if (op == ICodeOp::mul &&
        immediate > 0 &&
        (immediate & (immediate - 1)) == 0) {
        op = ICodeOp::sll;
        immediate = N - __builtin_clz(immediate) - 1;
    }
}
