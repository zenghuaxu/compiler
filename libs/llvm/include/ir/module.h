//
// Created by LENOVO on 2024/11/6.
//

#ifndef MODULE_H
#define MODULE_H

#include "../llvm.h"
#include "../llvmContext.h"

class Module {
    public:
    Module() {
        context = new LLVMContext();
    }

    [[nodiscard]] LLVMContextPtr getContext() const {
        return context;
    }

    void print(std::ostream &out) {
        context->print(out);
    }

    void mem2reg() {context->mem2reg();}
    void emit_bb() {context->emit_bb();}
    void delete_phi() {context->delete_phi();}

    void dce() {context->dce();}
    void lvn() {context->lvn();}
    private:
    LLVMContextPtr context;
};

#endif //MODULE_H
