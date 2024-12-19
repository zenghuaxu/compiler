//
// Created by LENOVO on 2024/11/6.
//

#ifndef LLVMCONTEXT_H
#define LLVMCONTEXT_H

#include "llvm.h"
#include "ir/globalValue.h"

class LLVMContext {
    friend class Module;
    friend class MipsManager;

    private:
    LLVMContext(): void_type(++current_id, this),
                bit_type(++current_id, this),
                char_type(++current_id, this),
                int_type(++current_id, this),
                main_func(nullptr) {}

    public:
    void SaveValue(ValuePtr value) {
        values.push_back(value);
    }
    void DeleteValue(ValuePtr value) {
        //TODO DO NOTHING?
    }
    void SaveUse(UsePtr use) {
        uses.push_back(use);
    }

    void SaveGlobal(GlobalValuePtr global_value) {
        globals.push_back(global_value);
    }
    void SaveFunction(FunctionPtr function) {
        functions.push_back(function);
    }
    void SaveMainFunction(FunctionPtr function) {
        main_func = function;
    }

    ValueReturnTypePtr getBitType() { return &bit_type; }
    ValueReturnTypePtr getIntType() { return &int_type; }
    ValueReturnTypePtr getCharType() { return &char_type; }
    ValueReturnTypePtr getVoidType() { return &void_type; }
    ValueReturnTypePtr getPointerType(ValueReturnTypePtr element_type) {
        if (pointerTypes.find(element_type) == pointerTypes.end()) {
            pointerTypes.emplace(element_type, *new PointerType(++current_id, this, element_type));
        }
        return &pointerTypes.at(element_type);
    }
    ValueReturnTypePtr toPointerType(ValueReturnTypePtr type) {
        if (typeid(*type) == typeid(ValueArrayType)) {
            return getPointerType(dynamic_cast<ValueArrayTypePtr>(type)->element_type);
        }
        // if (typeid(*type) == typeid(PointerType)) {
        //     return type;
        // }
        return getPointerType(type);
    }
    ValueReturnTypePtr getArrayType(ValueReturnTypePtr element_type, unsigned int length) {
        auto array = new ValueArrayType(++current_id, this, element_type, length);
        arrayTypes.push_back(array);
        return array;
    }

    void print(std::ostream &out);

    void mem2reg();

    void emit_bb();

    void delete_phi();

    void dce();

    void lvn();

private:
    std::vector<ValuePtr> values;
    std::vector<UsePtr> uses;
    std::vector<FunctionPtr> functions;
    FunctionPtr main_func;
    std::vector<GlobalValuePtr> globals;

    unsigned int current_id = 0;
    BitType bit_type;
    CharType char_type;
    IntType int_type;
    VoidType void_type;
    std::unordered_map<ValueReturnTypePtr, PointerType> pointerTypes;
    std::vector<ValueReturnTypePtr> arrayTypes;
};

#endif //LLVMCONTEXT_H
