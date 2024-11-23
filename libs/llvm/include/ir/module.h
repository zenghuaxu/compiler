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

    private:
    LLVMContextPtr context;
};

#endif //MODULE_H
