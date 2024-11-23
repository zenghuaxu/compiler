//
// Created by LENOVO on 2024/11/22.
//

#include <unordered_map>

#include "../include/mipsInst.h"
#include "../include/mipsReg.h"

void Tag::print(std::ostream &out) {
    out << name << ":";
}

std::unordered_map<RCodeOp, std::string> RCodeOpMap = {
    {RCodeOp::addu, "addu"},
    {RCodeOp::subu, "subu"},
    {RCodeOp::mul, "mul"},
    {RCodeOp::div, "div"},
    {RCodeOp::slt, "slt"},
    {RCodeOp::sle, "sle"},
    {RCodeOp::sgt, "sgt"},
    {RCodeOp::sge, "sge"},
    {RCodeOp::seq,"seq"},
    {RCodeOp::sne, "sne"},
};

void RCode::print(std::ostream &out) {
    if (op < RCodeOp::move) {
        out << "\t" << RCodeOpMap[op] << " ";
        rd->print(out);
        out << ", ";
        rs->print(out);
        out << ", ";
        rt->print(out);
    } else if (op == RCodeOp::move) {
        out << "\tmove ";
        rd->print(out);
        out << ", ";
        rs->print(out);
    } else if (op == RCodeOp::mod) {
        out << "\tdiv ";
        rs->print(out);
        out << ", ";
        rt->print(out);
        out << std::endl;
        out << "\tmfhi ";
        rd->print(out);
    }
}

std::unordered_map<ICodeOp, std::string> ICodeOpMap = {
    {ICodeOp::addiu, "addiu"},
    {ICodeOp::subiu, "subiu"},
    {ICodeOp::andi, "andi"},
    {ICodeOp::mul, "mul"},
    {ICodeOp::div, "div"},
};

void ICode::print(std::ostream &out) {
    if (op < ICodeOp::li) {
        out << "\t" << ICodeOpMap[op] << " ";
        rt->print(out);
        out << ", ";
        rs->print(out);
        out << ", ";
        if (offset) {
            offset->print(out);
        }
        else {
            out << immediate;
        }
    } else if (op == ICodeOp::li) {
        out << "\tli ";
        rt->print(out);
        out << ", ";
        if (offset) {
            offset->print(out);
        }
        else {
            out << immediate;
        }
    }
}

std::unordered_map<MemCodeOp, std::string> MemCodeOpMap = {
    {MemCodeOp::lbu, "lbu"},
    {MemCodeOp::lw, "lw"},
    {MemCodeOp::sb, "sb"},
    {MemCodeOp::sw, "sw"},
};

void MemCode::print(std::ostream &out) {
    out << "\t" << MemCodeOpMap[op] << " ";
    rt->print(out);
    out << ", ";
    if (offset) {
        offset->print(out);
    }
    out << "(";
    base->print(out);
    out << ")";
}

std::unordered_map<BranchCodeOp, std::string> BranchCodeOpMap = {
    {BranchCodeOp::beq, "beq"},
    {BranchCodeOp::bne, "bne"},
};

void BranchCode::print(std::ostream &out) {
    out << "\t" << BranchCodeOpMap[op] << " ";
    rs->print(out);
    out << ", ";
    rt->print(out);
    out << ", ";
    out << label_name;
}

void JumpCode::print(std::ostream &out) {
    out << "\tj " << label_name;
}

void JalCode::print(std::ostream &out) {
    out << "\tjal " + name;
}

void JrCode::print(std::ostream &out) {
    out << "\tjr ";
    ra->print(out);
}

void SysCallCode::print(std::ostream &out) {
    out << "\t" << "syscall";
}

void LaCode::print(std::ostream &out) {
    out << "\t" << "la ";
    rd->print(out);
    out << " " << label_name;
}