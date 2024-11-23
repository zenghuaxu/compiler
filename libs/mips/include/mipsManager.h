//
// Created by LENOVO on 2024/11/20.
//

#ifndef MIPSMANAGER_H
#define MIPSMANAGER_H
#include <vector>

#include "data.h"
#include "mips.h"
#include "mipsInst.h"
#include "translator.h"
#include "../../llvm/include/llvm.h"
#include "../../llvm/include/ir/module.h"

class MipsManager {
    friend class Translator;
    public:
    MipsManager(ModulePtr module);

    void mem_alloc(ValuePtr value, MemOffsetPtr mem);

    void translate();

    void print(std::ostream &out) {
        out << ".data:" << std::endl;
        for (auto it : data) {
            it->print(out);
            out << std::endl;
        }
        out << std::endl;
        out << ".text:" << std::endl;
        for (auto inst: insts) {
            inst->print(out);
            out << std::endl;
        }
    }

    private:
    ModulePtr module;
    std::vector<DataPtr> data;
    std::vector<MipsInstPtr> insts;

    SpRegPtr sp;
    ZeroRegPtr zero;
    RaRegPtr ra;
    std::vector<ARegPtr> areg;
    std::vector<VRegPtr> vreg;
    std::vector<TmpRegPtr> tmp;
    std::vector<SaveRegPtr> save;
    std::vector<SwapRegPtr> swap;

    std::unordered_map<ValuePtr, RegPtr> value_reg_map;
    TranslatorPtr translator;
};

#endif //MIPSMANAGER_H
