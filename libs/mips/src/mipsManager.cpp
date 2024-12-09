//
// Created by LENOVO on 2024/11/21.
//
#include "../include/mips.h"
#include "../include/mipsReg.h"
#include "../include/mipsManager.h"

#include "../../llvm/include/ir/module.h"

MipsManager::MipsManager(ModulePtr module):
    module(std::move(module)), translator(new Translator(this)) {
    for (int i = 0; i < TMP_NUM; i++) {
        tmp.emplace_back(new TmpReg(i));
    }
    for (int i = 0; i < SAVE_NUM; i++) {
        save.push_back(new SaveReg(i));
    }
    for (int i = 0; i < SWAP_NUM; i++) {
        swap.push_back(new SwapReg(i));
    }
    for (int i = 0; i < A_REG_NUM; i++) {
        areg.emplace_back(new AReg(i));
    }
    for (int i = 0; i < V_REG_NUM; i++) {
        vreg.emplace_back(new VReg(i));
    }
    sp = new SpReg();
    zero = new ZeroReg();
    ra = new RaReg();
}

void MipsManager::mem_alloc(ValuePtr value, MemOffsetPtr mem) {
    value_reg_map[value] = mem;
}

void MipsManager::translate() {
    for (auto it: module->getContext()->globals) {
        auto global_data = translator->translate(it);
        if (typeid(*global_data) == typeid(WordData)) {
            word_data.push_back(global_data);
        }
        else {
            data.push_back(global_data);
        }
    }

    translator->translate(module->getContext()->main_func, insts);
    for (auto func: module->getContext()->functions) {
        translator->translate(func, insts);
    }
}
